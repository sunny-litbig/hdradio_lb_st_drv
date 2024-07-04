#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QDesktopWidget>
#include <QApplication>

#include "DBusInterface.h"
#include "RadioInterface.h"
#include "QMLInterface.h"

int g_appID = -1;
int g_debug = 0;

#define APP_WIDTH	1024
#define APP_HEIGHT 	600
#define OSD_HEIGHT 	65

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    for (int i = 1; i < argc; i++) {
        QString argument = argv[i];
        argument = argument.toUpper();

        if (argument == "--NO-V4L2") {
            // Not used
        } else if (argument == "--MOUSE") {
            // Not used
        } else if (argument == "--DEBUG") {
			g_debug = 1;
        } else if (argument.left(5) == "--ID=") {
            g_appID = argument.mid(5).toInt();
            qDebug("Application ID: %d", g_appID);
        } else if (argument == "--REAR") {
            // Not used
        }
    }

    QMLInterface qmlInterface;
    DBusInterface dbusInterface;
    RadioInterface radioInterface;

    qmlInterface.connectDBusInterface(&dbusInterface);
    qmlInterface.connectRadioInterface(&radioInterface);

    QQmlApplicationEngine engine;
	QQuickWindow* window;
#ifdef QT_DEBUG
    engine.rootContext()->setContextProperty("DEBUG_MODE", true);
#else
    engine.rootContext()->setContextProperty("DEBUG_MODE", false);
#endif
    engine.rootContext()->setContextProperty("Native", &qmlInterface);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
	QObject* tmp = engine.rootObjects().value(0);
	window = qobject_cast<QQuickWindow*>(tmp);

	QDesktopWidget *desktop = QApplication::desktop();

	int displayNumber = 0;
	int x,y,width,height;
	char *env = getenv("IVI_DISPLAY_NUMBER");
	if (env != NULL)
	{
		displayNumber = atoi(env);
	}

	QRect screenRect = desktop->screenGeometry(displayNumber);

	x = (screenRect.width() - APP_WIDTH) / 2;
	y = (screenRect.height() - APP_HEIGHT) / 2 + OSD_HEIGHT;
	width = APP_WIDTH;
	height = y == OSD_HEIGHT ? APP_HEIGHT - OSD_HEIGHT : (screenRect.height() + APP_HEIGHT)/2 - OSD_HEIGHT ;
	window->setGeometry(x, y, width, height);
	qmlInterface.sendInitInfo();
    return app.exec();
}
