package perienv

import (
    "android/soong/android"
    "android/soong/cc"
    //"fmt"
)

func globalFlags(ctx android.BaseContext) []string {
    var cflags []string

    /////////////////////////////////////////////////////////////////
    if ctx.AConfig().Getenv("TARGET_PRODUCT") == "tcc897x" {
        cflags = append(cflags, "-DUSE_TCC897X")

        //cflags = append(cflags, "-DTCC897X_LCN20_BOARD")
        //cflags = append(cflags, "-DTCC897X_LCN30_BOARD")
    }
    if ctx.AConfig().Getenv("TARGET_PRODUCT") == "tcc802x" {
        cflags = append(cflags, "-DUSE_TCC802X")
        //cflags = append(cflags, "-DTCC802X_BOARD")
    	//cflags = append(cflags, "-DTCC802X_EVM21_BOARD")
    }
    if ctx.AConfig().Getenv("TARGET_PRODUCT") == "tcc803x" || 
        ctx.AConfig().Getenv("TARGET_PRODUCT") == "car_tcc803x" ||
        ctx.AConfig().Getenv("TARGET_PRODUCT") == "car_tcc803x_arm64" ||
        ctx.AConfig().Getenv("TARGET_PRODUCT") == "car_tcc803xp_arm64" {
        cflags = append(cflags, "-DUSE_TCC803X")
        //cflags = append(cflags, "-DTCC8030_BOARD")
    	//cflags = append(cflags, "-DTCC8031_BOARD")
    }
    if ctx.AConfig().Getenv("TARGET_PRODUCT") == "car_tcc8059_arm64" {
        cflags = append(cflags, "-DUSE_TCC805X")

        cflags = append(cflags, "-DTCC8059_MAIN_BOARD")
        //cflags = append(cflags, "-DTCC8059_SUB_BOARD")
    }
    if ctx.AConfig().Getenv("TARGET_PRODUCT") == "car_tcc8050_arm64" {
        cflags = append(cflags, "-DUSE_TCC805X")

        //cflags = append(cflags, "-DTCC8053_MAIN_BOARD")
        //cflags = append(cflags, "-DTCC8053_SUB_BOARD")
        cflags = append(cflags, "-DTCC8050_MAIN_BOARD")
        //cflags = append(cflags, "-DTCC8050_SUB_BOARD")
    }
    /////////////////////////////////////////////////////////////////

    //fmt.Println("[TARGET_PRODUCT] =========> "+ctx.AConfig().Getenv("TARGET_PRODUCT"))


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
    android.RegisterModuleType("perienv", Factory)
}

func Factory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, myDefaults)
    return module
}

