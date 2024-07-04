package tunerenv

import (
    "android/soong/android"
    "android/soong/cc"
)

func globalFlags(ctx android.BaseContext) []string {
    var cflags []string
    return cflags
}

func myDefaults(ctx android.LoadHookContext) {
    type props struct {
        Cflags []string
    }

    p := &props{}
    p.Cflags = globalFlags(ctx)

    ctx.AppendProperties(p)
}

func init() {
    android.RegisterModuleType("tunerenv", Factory)
}

func Factory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, myDefaults)
    return module
}

