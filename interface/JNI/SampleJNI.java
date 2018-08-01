public class SampleJNI{

    public native int SampleInit();
    public native void SampleExit();
    public native int SampleSetCallback();

	public int onSample_Callback(String buf, int size)
	{
	//	Log.d("SwinterfaceJNI", "printf:" + buf + size);
		System.out.println("SampleJNI:" + buf + size);
		return 0;
	}
    
    /*static {System.loadLibrary("swinterface");}
    
    public static void main(String args[]) {
        SwinterfaceJNI myJNI = new SwinterfaceJNI();
        
        myJNI.SwNetworkInit();
        myJNI.SwNetworkExit();
    }*/
    
}
