package co

import (
    "android/soong/android"
    "android/soong/cc"
)

func init() {
    android.RegisterModuleType("co", factory)
}

func factory() android.Module {
    module := cc.DefaultsFactory()
    return module
}

/**************************************************************
* Call from outside
***************************************************************/
func CGray() string {return "\033[1;30m"}
func CRed() string {return "\033[1;31m"}
func CGreen() string {return "\033[1;32m"}
func CYellow() string {return "\033[1;33m"}
func CBlue() string {return "\033[1;34m"}
func CMagenta() string {return "\033[1;35m"}
func CCyan() string {return "\033[1;36m"}
func CWhite() string {return "\033[1;37m"}
func CEnd() string {return "\033[0m"}

