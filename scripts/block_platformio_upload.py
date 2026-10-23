# PlatformIO/SCons hook: only the dedicated app1 procedure is safe here.
Import("env")

def refuse_upload(source, target, env):
    raise RuntimeError("Standard upload would overwrite ESPINFO. Use the Slot B app-only procedure in README.md.")

env.AddPreAction("upload", refuse_upload)
