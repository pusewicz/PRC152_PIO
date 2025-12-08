# import subprocess
# Import("env")
# # my_flags = env.ParseFlags(env['BUILD_FLAGS'])
# # defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}
# # env.Replace(PROGNAME="PRC152-N%s" % (defines.get("PIO_SRC_TAG")))
# framework = env["PIOFRAMEWORK"][0]
# env_name = str(env["PIOENV"])

# print("Framework: %s Environment: %s" % (framework, env_name))
# env.Replace(PROGNAME="PRC152-N %s" % (defines.get("PIO_SRC_TAG")))

Import("env")

env.Replace(PROGNAME="PRC152-N%s" % env.GetProjectOption("custom_prog_version"))