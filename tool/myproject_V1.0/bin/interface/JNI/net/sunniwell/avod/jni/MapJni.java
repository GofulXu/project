package net.sunniwell.avod.jni;

public class MapJni{

	private MapJni() {
	}

    public native int SwNetworkInit();
    public native void SwNetworkExit();
    public native int SwNetworkSetCallback();
    public native int SwClockSettime(int year, int mon, int day, int hour, int min, int sec);

	public int onSwnetwork_Callback(String buf, int size)
	{
	//	Log.d("SwinterfaceJNI", "printf:" + buf + size);
		System.out.println("SwinterfaceJNI:" + buf + size);
		return 0;
	}
    
    /*static {System.loadLibrary("swinterface");}
    
    public static void main(String args[]) {
        SwinterfaceJNI myJNI = new SwinterfaceJNI();
        
        myJNI.SwNetworkInit();
        myJNI.SwNetworkExit();
    }*/
    
}
