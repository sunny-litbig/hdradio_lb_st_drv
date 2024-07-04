package tcradioenv

import (
    "android/soong/android"
    "android/soong/cc"
    //"fmt"
    //"path"
)


func globalDefaults(ctx android.BaseContext) ([]string, []string, []string) {
	var cflags []string
	var includeDirs []string
	var sharedlibs []string

	/* HD Shared Includes */
	includeDirs = append(includeDirs, GetRadioPath()+"hd/src/api")

	/* Peri Shared Includes */
	includeDirs = append(includeDirs, GetRadioPath()+"peri/src")
	includeDirs = append(includeDirs, GetRadioPath()+"peri/src/conf")
	includeDirs = append(includeDirs, GetRadioPath()+"peri/src/tuner")
	includeDirs = append(includeDirs, GetRadioPath()+"peri/src/i2s")

	/* MW Shared Includes */
	includeDirs = append(includeDirs, GetRadioPath()+"mw/src")
	includeDirs = append(includeDirs, GetRadioPath()+"mw/src/hd")
	includeDirs = append(includeDirs, GetRadioPath()+"mw/src/rds")

	/* Hal Shared Includes */
	includeDirs = append(includeDirs, GetRadioPath()+"hal/src")
	includeDirs = append(includeDirs, GetRadioPath()+"hal/src/conf")
	includeDirs = append(includeDirs, GetRadioPath()+"hal/src/fifo")

	/* Shared CFlag */
	cflags = append(cflags,
		"-Werror",

		"-Wno-null-arithmetic",
		"-Wno-null-conversion",

		"-Wno-unused-function",
		"-Wno-unused-variable",
		"-Wno-unused-parameter",
		"-Wno-unused-private-field",

		"-Wno-pointer-sign",
		"-Wno-pointer-arith",
		"-Wno-pointer-integer-compare",

		"-Wno-switch",
		"-Wno-format",
		"-Wno-visibility",
		"-Wno-self-assign",
		"-Wno-array-bounds",
		"-Wno-sign-compare",
		"-Wno-int-conversion",
		"-Wno-missing-braces",
		"-Wno-macro-redefined",
		"-Wno-enum-conversion",
		"-Wno-incompatible-pointer-types",
		"-Wno-tautological-pointer-compare",
		"-Wno-implicit-function-declaration",

		"-pthread",
	)

    return cflags, includeDirs, sharedlibs
}

func deviceFlags(ctx android.BaseContext) []string {
    var cflags []string
    return cflags
}

func hostFlags(ctx android.BaseContext) []string {
    var cflags []string
    return cflags
}

func myDefaults(ctx android.LoadHookContext) {
    type props struct {
        Target struct {
            Android struct {
                Cflags []string
                Enabled *bool
            }
            Host struct {
                Enabled *bool
            }
            Linux struct {
                Cflags []string
            }
            Darwin struct {
                Cflags []string
            }
        }
        Cflags []string
        Include_dirs []string
        Shared_libs []string
    }

    p := &props{}
    p.Cflags, p.Include_dirs, p.Shared_libs = globalDefaults(ctx)
    p.Target.Android.Cflags = deviceFlags(ctx)
    h := hostFlags(ctx)
    p.Target.Linux.Cflags = h
    p.Target.Darwin.Cflags = h

    ctx.AppendProperties(p)
}

func init() {
    android.RegisterModuleType("tcradioenv", myDefaultsFactory)
}

func myDefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, myDefaults)
    return module
}


/**************************************************************
* Call from outside
***************************************************************/
func GetRadioTuner() string {
	return "s0tuner"
	//return "m0tuner"
	//return "x0tuner"
}

func GetRadioHDRadio() string {
	return "USE_HDRADIO"
	return ""
}

func GetRadioConfig() string {
	return "USE_TELECHIPS_EVB"
	return ""
}

func GetRadioPath() string {
	//return "hardware/interfaces/broadcastradio/tcc/driver/tc-radio/"
	return "hardware/telechips/radio/tc-radio/"
}

