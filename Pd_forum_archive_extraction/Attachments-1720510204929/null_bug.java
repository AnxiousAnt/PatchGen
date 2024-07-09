// this class crashes pd

import com.cycling74.max.*;

// here's our class
public class null_bug extends MaxObject {
	
	// this gets called on init of the max/pd object [RmiClient]
	public null_bug() {
		int numOutlets = 1;	
		int numInlets = 1;	
		declareIO(numInlets,numOutlets);
	}

	public void bang() {
			try {
				String outputList[] = new String[2];
				outputList[0] = "test";
				outlet(0, outputList);
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
	}
}