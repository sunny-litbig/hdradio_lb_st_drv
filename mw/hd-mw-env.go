package hdmwenv

import (
    "android/soong/android"
    "android/soong/cc"
    /* SOURCE */ "fmt"
    /* SOURCE */ "android/soong/tcradioenv"
    /* SOURCE */ "android/soong/co"
)

func globalDefaults(ctx android.BaseContext) ([]string, []string, []string) {
    var cflags []string
    var includeDirs []string
    var sharedlibs []string

    /* SOURCE */
    if tcradioenv.GetRadioHDRadio() == "USE_HDRADIO" {
        cflags = append(cflags, "-DUSE_HDRADIO")
        fmt.Println(co.CYellow()+"[MW-ENV] USE_HDRADIO"+co.CEnd())
    }

    return cflags, includeDirs, sharedlibs
}

func myDefaults(ctx android.LoadHookContext) {
    type props struct {
        Cflags []string
        Include_dirs []string
        Shared_libs []string
    }

    p := &props{}
    p.Cflags, p.Include_dirs, p.Shared_libs = globalDefaults(ctx)

    ctx.AppendProperties(p)
}

func init() {
    android.RegisterModuleType("hdmwenv", Factory)
}

func Factory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, myDefaults)
    return module
}
