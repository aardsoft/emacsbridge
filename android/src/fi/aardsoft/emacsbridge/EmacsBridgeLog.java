package fi.aardsoft.emacsbridge;

class EmacsBridgeLog {
    public static native void jDebug(String message);
    public static native void jInfo(String message);
    public static native void jWarning(String message);
    public static native void jCritical(String message);
}
