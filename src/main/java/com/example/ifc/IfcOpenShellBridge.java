package com.example.ifc;

public final class IfcOpenShellBridge {
    private static volatile boolean loaded;

    private IfcOpenShellBridge() {
    }

    public static void ensureLoaded() {
        if (!loaded) {
            synchronized (IfcOpenShellBridge.class) {
                if (!loaded) {
                    System.loadLibrary("ifcglb_jni");
                    loaded = true;
                }
            }
        }
    }

    public static native int convertIfcToGlb(String inputPath, String outputPath, String optionsJson);
}
