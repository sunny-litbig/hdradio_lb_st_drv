package halenv

import (
	"android/soong/android"
	"android/soong/cc"
	"fmt"
	/* SOURCE */ "android/soong/tcradioenv"
	/* SOURCE */ "android/soong/co"
)

func globalDefaults(ctx android.BaseContext) ([]string, []string, []string) {
	var cflags []string
	var includeDirs []string
	var sharedlibs []string

	/* SOURCE */
	if tcradioenv.GetRadioTuner() == "s0tuner" {
		fmt.Println(co.CGreen()+"[HAL-ENV] s0tuner"+co.CEnd())
		fmt.Println(co.CGreen()+"[HAL-ENV] * libsi4796xdriver"+co.CEnd())
		fmt.Println(co.CGreen()+"[HAL-ENV] * USE_S0_TUNER"+co.CEnd())

		sharedlibs = append(sharedlibs, "libsi4796xdriver")
		cflags = append(cflags, "-DUSE_S0_TUNER")
		includeDirs = append(includeDirs, tcradioenv.GetRadioPath()+"tuner/src/si479xx")
	}else if tcradioenv.GetRadioTuner() == "m0tuner" {
		fmt.Println(co.CGreen()+"[HAL-ENV] m0tuner"+co.CEnd())
		fmt.Println(co.CGreen()+"[HAL-ENV] * libmax2175driver"+co.CEnd())
		fmt.Println(co.CGreen()+"[HAL-ENV] * USE_M0_TUNER"+co.CEnd())

		sharedlibs = append(sharedlibs, "libmax2175driver")
		cflags = append(cflags, "-DUSE_M0_TUNER")
		includeDirs = append(includeDirs, tcradioenv.GetRadioPath()+"tuner/src/max2175")
	}else if tcradioenv.GetRadioTuner() == "x0tuner" {
		fmt.Println(co.CGreen()+"[HAL-ENV] x0tuner"+co.CEnd())
		fmt.Println(co.CGreen()+"[HAL-ENV] * libx0tunerdriver"+co.CEnd())
		fmt.Println(co.CGreen()+"[HAL-ENV] * USE_X0_TUNER"+co.CEnd())

		sharedlibs = append(sharedlibs, "libx0tunerdriver")
		cflags = append(cflags, "-DUSE_X0_TUNER")
		includeDirs = append(includeDirs, tcradioenv.GetRadioPath()+"tuner/src/x0tuner")
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
    android.RegisterModuleType("halenv", Factory)
}

func Factory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, myDefaults)
    return module
}


/**************************************************************
* Call from outside
***************************************************************/
func EnvDefault(ctx android.BaseContext, key string, defaultValue string) string {
	ret := ctx.AConfig().Getenv(key)
	if ret == "" {
		return defaultValue
	}
	return ret
}

func EnvTrue(ctx android.BaseContext, key string) bool {
	return ctx.AConfig().Getenv(key) == "true"
}

func EnvFalse(ctx android.BaseContext, key string) bool {
	return ctx.AConfig().Getenv(key) == "false"
}

func EnvTrueOrDefault(ctx android.BaseContext, key string) bool {
	return ctx.AConfig().Getenv(key) != "false"
}
