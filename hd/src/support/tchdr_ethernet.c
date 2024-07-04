/*******************************************************************************

*   FileName : tchdr_ethernet.c

*   Copyright (c) Telechips Inc.

*   Description : TC HD Radio framework ethernet APIs and definitions
                  (Provides API to access ethernet device driver)

********************************************************************************
*
*   TCC Version 1.0

This source code contains confidential information of Telechips.

Any unauthorized use without a written permission of Telechips including not
limited to re-distribution in source or binary form is strictly prohibited.

This source code is provided "AS IS" and nothing contained in this source code
shall constitute any express or implied warranty of any kind, including without
limitation, any warranty of merchantability, fitness for a particular purpose
or non-infringement of any patent, copyright or other third party intellectual
property right.
No warranty is made, express or implied, regarding the information's accuracy,
completeness, or performance.

In no event shall Telechips be liable for any claim, damages or other
liability arising from, out of or in connection with this source code or
the use in the source code.

This source code is provided subject to the terms of a Mutual Non-Disclosure
Agreement between Telechips and Company.
*
*******************************************************************************/
/***************************************************
*		Include 			   					*
****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifdef DEBUG_DRV
#include <sys/time.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

#include "hdrCore.h"
#include "tchdr_common.h"
#include "tchdr_log.h"
#include "tchdr_ethernet.h"

/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*             Local preprocessor                   *
****************************************************/
#ifdef DEBUG_DRV

#define RX_BYTE_REPORT_COUNT	(10000000)

static U64 total_bytes_received=0;
static U64 totalMbtally=RX_BYTE_REPORT_COUNT;
static struct timeval tv2;
static struct timeval tv1;
static F64 startTime=0;
static F64 dTime2;
static F64 elapsedTime;
F32 readRate;

static S8 RxprintBuffer[1024];
static S8 TxprintBuffer[2048];

#endif

#define MAX_OPEN_PORTS      (4U)


/***************************************************
*           Local type definitions                 *
****************************************************/
typedef enum {
    ePORT_SOCKET_LISTENING,
    ePORT_SOCKET_INACTIVE,
    ePORT_SOCKET_ACTIVE
}ePORT_SOCKET_STATE_t;

typedef struct {
    U32 port;
    ePORT_SOCKET_STATE_t state;
    pthread_t listenerThread;
    sem_t socketClosedSem;
	S32 socketrun;
    S32 socketListener;
    S32 socketClient;
}stPORT_SOCKET_t;

/***************************************************
*           Local constant definitions              *
****************************************************/
static stPORT_SOCKET_t stPortSocket[MAX_OPEN_PORTS];

/***************************************************
*          Local function prototypes               *
****************************************************/
static stPORT_SOCKET_t* getPortSocket(U32 port);
static void closePortSocket(stPORT_SOCKET_t* argSocket);
static void resetPortSocket(stPORT_SOCKET_t* argSocket);
static void* portListener(void *arg);

/***************************************************
*			function definition				*
****************************************************/
S32 tchdr_ethClose(U32 port)
{
	S32 ret = -1;
	U32 i;
	for(i = 0; i < MAX_OPEN_PORTS;i++) {
		if(stPortSocket[i].port == port) {
			(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Closing Port: %d, Socket: %d\n", stPortSocket[i].port, stPortSocket[i].socketClient);
			closePortSocket(&stPortSocket[i]);
			ret = 0;
			break;
		}
	}
#ifdef DEBUG_DRV
	startTime=0;
	//	total_bytes_received=0;
	totalMbtally=RX_BYTE_REPORT_COUNT;
#endif
	return ret;
}

S32 tchdr_ethReset(U32 port)
{
	S32 ret = -1;
	U32 i;
	for(i = 0; i < MAX_OPEN_PORTS;i++) {
		if(stPortSocket[i].port == port) {
			(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Reset Port: %d, Socket: %d\n", stPortSocket[i].port, stPortSocket[i].socketClient);
			resetPortSocket(&stPortSocket[i]);
			ret = 0;
			break;
		}
	}
#ifdef DEBUG_DRV
	startTime=0;
	//	total_bytes_received=0;
	totalMbtally=RX_BYTE_REPORT_COUNT;
#endif
	return ret;
}

S32 tchdr_ethOpen(U32 port, U32 type)
{
	U32 i;
	S32 ret=0;
	pthread_attr_t attr;
	stPORT_SOCKET_t* portSocket = getPortSocket(port);

	if(portSocket != NULL) {
		(*pfnHdrLog)(eTAG_ETH, eLOG_WRN,"Port: %d already Open", port);
		// Port is already open
		ret = -1;
	}
	else {
		for(i = 0; i < MAX_OPEN_PORTS;i++) {
			if(stPortSocket[i].port == 0U) {
				portSocket = &stPortSocket[i];
				portSocket->port = port;
				portSocket->state = ePORT_SOCKET_INACTIVE;
				portSocket->socketrun = 0;
				portSocket->socketListener = -1;
				portSocket->socketClient = -1;
				(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Created stPortSocket for Port: %d, Type: %d\n", port, type);
				break;
			}
		}

		if(portSocket == NULL) {
			(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "PortSocket is null. Could not create listener.\n");
			ret = -1;
		}
	}

	if(ret == 0) {
		ret = pthread_attr_init(&attr);
	}

	if(ret == 0) {
		ret = pthread_attr_setdetachstate(&attr, (S32)PTHREAD_CREATE_DETACHED);
	}

	if(ret == 0) {
		ret = pthread_create(&portSocket->listenerThread, &attr, &portListener, (void*)portSocket);
		if(ret != 0) {
			(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "Error:could not create listener thread.\n");
		}
	}

	if(ret == 0) {
		ret = pthread_attr_destroy(&attr);
	}

	if(ret != 0) {
		ret = -1;
	}

	UNUSED(type);
	return ret;
}

S32 tchdr_ethRead(U32 port, U8* buffer, U32 length)
{
	S32 ret=0;
	SLONG rxLen;
	stPORT_SOCKET_t* portSocket = getPortSocket(port);

	if(portSocket == NULL) {
		(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "ethRead() failed to get the TCP/IP socket. Port[%d]\n", port);
		ret = -2;
	}
	else if(portSocket->socketClient == -1) {
		// Not connected
		(void)usleep(10000);
		ret = -1;
	}
	else {
		rxLen = recv(portSocket->socketClient, (void*)buffer, length, (S32)MSG_WAITALL);

		if(rxLen == -1){
			(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "TCP/IP Receive error: %d\n", errno);
			(void)sem_post(&portSocket->socketClosedSem);
			(void)usleep(10000);
			ret = -1;
		}
		else if(rxLen == 0) {
			(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "Client closed the connection.\n");
			(void)sem_post(&portSocket->socketClosedSem);
			(void)usleep(10000);
			ret = -1;
		}
		else if(rxLen != (*stCast.u32toslong)(length)) {
			(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "Bad message received, %ld bytes\n", rxLen);
			(void)sem_post(&portSocket->socketClosedSem);
			(void)usleep(10000);
			ret = -1;
		}
		else {
#ifdef DEBUG_DRV
			S32 i;
			for(i=0; i<dw && i<128; i++)
			{
				sprintf(&RxprintBuffer[6*i], " 0x%2.2x,",buffer[i]);
			}
			RxprintBuffer[6*dw]=0; /* set end of string */
			(*pfnHdrLog)(eTAG_ETH, eLOG_DBG,  "Read %d bytes: \"%s\"", dw, RxprintBuffer);
			total_bytes_received+=dw;
			if(total_bytes_received > totalMbtally)
			{
				totalMbtally=total_bytes_received+RX_BYTE_REPORT_COUNT;
				gettimeofday(&tv2, NULL);
				dTime2 = (F64)tv2.tv_sec + ((F64)tv2.tv_usec * 1.0E-06);
				elapsedTime = (F64)((F32)(dTime2 - startTime));
				readRate = (F32)(total_bytes_received / elapsedTime);
				//(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Average Read Rate = %3.3f MBytes/sec\n", readRate/RX_BYTE_REPORT_COUNT);
				(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Received %12.12llu bytes\n", total_bytes_received);
			}
#else
//			(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Received %d bytes\n", dw);
#endif
		}
	}

    return ret;
}

S32 tchdr_ethWrite(U32 port, const U8 *buffer, S32 num_bytes)
{
	S32 ret = 0;
	S32 flags = (S32)MSG_NOSIGNAL;	//prevent SIGPIPE
	SLONG bytesWritten;
	U32 totalBytesSent=0;
#ifdef DEBUG_DRV
	F64 dRxTime, dTxTime, dDiffTime;
	S32 i;
#endif

	stPORT_SOCKET_t* portSocket = getPortSocket(port);

	if(portSocket == NULL) {
		(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "ethWrite() failed to get the TCP/IP socket. Port[%d]\n", port);
		ret = -2;
	}
	else if(portSocket->socketClient == -1) {
		// Not connected
		ret = -1;
	}
	else {
		//make sure all bytes are written out
		while((S32)totalBytesSent < num_bytes) {
			bytesWritten = send(portSocket->socketClient, &buffer[totalBytesSent], (size_t)num_bytes-(size_t)totalBytesSent, flags);
			if(bytesWritten < 0) {
				S32 error_number = errno;
				(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "tchdr_ethWrite() failed with error code : %d\n" , errno);
				switch(error_number) {
					case EBADF:
						(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The socket argument is not a valid file descriptor\n");
						break;
					case ENOTSOCK:
						(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The descriptor socket is not a socket\n");
						break;
					case EWOULDBLOCK:
						(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "Nonblocking mode has been set on the socket, and the read operation would block\n");
						break;
					case EINTR:
						(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The operation was interrupted by a signal before any data was read\n");
						break;
					case ENOTCONN:
						(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "You never connected this socket\n");
						break;
					case EMSGSIZE:
						(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The socket type requires that the message be sent atomically, but the message is too large for this to be possible\n");
						break;
					case ENOBUFS:
						(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "There is not enough internal buffer space available\n");
						break;
					case EPIPE:
						(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "This socket was connected but the connection is now broken\n");
						break;
					default:
						(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "send unknown error\n");
						break;
				}
				// inform Listener thread that the connection is closed
				(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "Closing Socket: %d\n", portSocket->socketClient);
				(void)sem_post(&portSocket->socketClosedSem);
				(void)usleep(10000);
				ret = -1;
				break;
			}
			totalBytesSent += (U32)bytesWritten;
		}
	}

	if(ret == 0) {
		ret = (S32)totalBytesSent;
#ifdef DEBUG_DRV
		(void)memset((void*)TxprintBuffer,0,sizeof(TxprintBuffer));
		for(i=0; i<totalBytesSent && i<32; i++)
		{
			sprintf(&TxprintBuffer[6*i], " 0x%2.2x,",buffer[i]);
		}
		(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Wrote %d bytes: \"%s\"", totalBytesSent, TxprintBuffer);
#else
//		(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Sent %d bytes", totalBytesSent);
#endif
	}
    return ret;
}

static void* portListener(void* arg)
{

	S32 sockListener;
	struct sockaddr_in saListener;

	stPORT_SOCKET_t* socket_ = (stPORT_SOCKET_t*)arg;
	S32 sockfd = -1;
	S32 opt = 1;
	U32 sin_size;
	struct sockaddr_in ipIncoming;

	if(socket_ != NULL) {
		S32 ret = 0;
		S32 error_number;
		// Init listen socket to defaults
		(void)memset((void*)&saListener, 0x00, sizeof(struct sockaddr_in));
		saListener.sin_family = AF_INET;
		//saListener.sin_addr.s_addr = INADDR_ANY;	// Codesonar: Useless Assignment Warning
		saListener.sin_port = htons((*stCast.u32tou16)(socket_->port));

		// Create and bind the listen socket
		sockListener = socket((S32)AF_INET, (S32)SOCK_STREAM, (S32)IPPROTO_TCP);
		if(sockListener == -1) {
			error_number = errno;
			switch(error_number) {
				case EPROTONOSUPPORT:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The protocol or style is not supported by the namespace specified\n");
					break;
				case EMFILE:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The process already has too many file descriptors open\n");
					break;
				case ENFILE:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The system already has too many file descriptors open\n");
					break;
				case EACCES:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The process does not have the privilege to create a socket of the specified style or protocol\n");
					break;
				case ENOBUFS:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The system ran out of internal buffer space\n");
					break;
				default:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "Unable to create listener socket\n");
					break;
			}
			ret = -1;
		}

		if((ret == 0) && (setsockopt(sockListener, SOL_SOCKET, SO_REUSEADDR, (void*)(&opt), (socklen_t)sizeof(S32)) == -1)) {
			S8 errBuf[HDR_ERR_BUF_SIZE]={0,};
			(void)strerror_r(errno, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "setsockopt() failed: %s\n", errBuf);
			(void)close(sockListener);
			ret = -1;
		}

		if((ret == 0) && (bind(sockListener, (struct sockaddr *)&saListener, (socklen_t)sizeof(struct sockaddr_in)) == -1)) {
			error_number = errno;
			switch (error_number){
				case EBADF:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The socket argument is not a valid file descriptor\n");
					break;
				case ENOTSOCK:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The descriptor socket is not a socket\n");
					break;
				case EADDRNOTAVAIL:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The specified address is not available on this machine\n");
					break;
				case EADDRINUSE:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "Some other socket is already using the specified address\n");
					break;
				case EINVAL:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The socket socket already has an address\n");
					break;
				case EACCES:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "You do not have permission to access the requested address\n");
					break;
				default:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "bind unknown error\n");
					break;
			}
			(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "Unable to bind ,Port %d may already be in use\n", socket_->port);
			(void)close(sockListener);
			ret = -1;
		}

		if((ret == 0) && (listen(sockListener, SOMAXCONN) == -1)) {
			error_number = errno;
			switch (error_number){
				case EBADF:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The socket argument is not a valid file descriptor\n");
					break;
				case ENOTSOCK:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The descriptor socket is not a socket\n");
					break;
				case EOPNOTSUPP:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "The descriptor socket does not support this operation\n");
					break;
				default:
					(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "listen unknown error\n");
					break;
			}
			(*pfnHdrLog)(eTAG_ETH, eLOG_ERR, "Unable to listen\n");
			(void)close(sockListener);
			ret = -1;
		}

		if(ret == 0) {
			socket_->socketListener = sockListener;

			// Accept incoming connections and pass them to connection threads - This is a BLOCKING call
			sin_size = (U32)sizeof(struct sockaddr_in);

			(void)sem_init(&socket_->socketClosedSem, 0, 0);
			socket_->socketClient = -1;

			socket_->socketrun = 1;
			socket_->state = ePORT_SOCKET_ACTIVE;

			while(socket_->socketrun > 0) {
				(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Waiting for Connection on Port %d... \n", socket_->port);
				sockfd = accept(sockListener, (struct sockaddr *)&ipIncoming, &sin_size);

				if(sockfd < 0){
					(void)sleep(1);
					continue;
				}

				socket_->state = ePORT_SOCKET_LISTENING;
				socket_->socketClient = sockfd;

				(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Socket %d Connected, (Port %d)\n", socket_->socketClient, socket_->port);
				(void)sem_wait(&socket_->socketClosedSem);

				socket_->socketClient = -1;
				(void)close(sockfd);
				(void)sem_init(&socket_->socketClosedSem, 0, 0);
				(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Socket %d Closed (Port %d)\n", sockfd, socket_->port);
			}

			//(*pfnHdrLog)(eTAG_ETH, eLOG_DBG, "Incoming connection from address: %s, port: 0x%x\n", inet_ntoa(ipIncoming.sin_addr),ntohs(ipIncoming.sin_port));
			socket_->socketrun = -1;
			(void)sem_destroy(&socket_->socketClosedSem);
		}
	}

	return NULL;
}

static stPORT_SOCKET_t* getPortSocket(U32 port)
{
	stPORT_SOCKET_t* ret = NULL;
	U32 i;
	for (i = 0; i < MAX_OPEN_PORTS;i++){
		if (stPortSocket[i].port == port){
			ret = &stPortSocket[i];
			break;
		}
	}
	return ret;
}

static void resetPortSocket(stPORT_SOCKET_t* argSocket)
{
	if(argSocket != NULL) {
		(void)close(argSocket->socketClient);
		(void)close(argSocket->socketListener);
		(void)memset((void*)argSocket, 0, sizeof(stPORT_SOCKET_t));
	}
}

static void closePortSocket(stPORT_SOCKET_t* argSocket)
{
	S32 val;
	S32 rc;
	if(argSocket != NULL) {
		argSocket->state = ePORT_SOCKET_INACTIVE;
		argSocket->socketrun = 0;
		rc = sem_getvalue(&argSocket->socketClosedSem, &val);
		if((rc == 0) && (val == 0)) {
			(void)sem_post(&argSocket->socketClosedSem);
		}
		(void)shutdown(argSocket->socketClient, (S32)SHUT_RDWR);
		(void)shutdown(argSocket->socketListener, (S32)SHUT_RDWR);
		(void)close(argSocket->socketClient);
		(void)close(argSocket->socketListener);
		(void)memset((void*)argSocket, 0, sizeof(stPORT_SOCKET_t));
	}
}

