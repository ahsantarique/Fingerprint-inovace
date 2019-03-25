/*
created by Ahsan
*/
import java.io.*;
import java.nio.ByteBuffer;
import java.util.*;

public class BinText{
	 
	public static ArrayList <Byte> a = new ArrayList<Byte>();
	public static ArrayList <Byte> byteFile = new ArrayList<Byte>();
	public static void main(String args[]) throws IOException{
		DataInputStream  input = new DataInputStream(new FileInputStream(System.getProperty("user.dir")+"/LOG.txt"));
		while(input.available() > 0){
			//System.out.println("loop er moddhe");
			byte b = input.readByte();
			a.add(b);
			//String x= String.format("%02x", b & 0xFF);
			//System.out.println(x);
		}
		input.close();
		
		int n= a.size();
		byte[] bytearray = new byte[a.size()];
		int idx=0;
		for (Byte b:a)bytearray[idx++]=b.byteValue();
		for(int i = 0; i < n; i+=22){
			byte[] deviceID = new byte[4];
			new Random().nextBytes(deviceID);
			for (byte d:deviceID) byteFile.add(d);
			for(int j = 0; j < 22; j++){
				byteFile.add(a.get(i+j));
			}
			
			int year = a.get(i).intValue();
			int month = a.get(i+1).intValue();
			int date = a.get(i+2).intValue();
			int hour = a.get(i+3).intValue();	
			int minute = a.get(i+4).intValue();
			int second = a.get(i+5).intValue();	
			int cf = a.get(i+6).intValue();
			ByteBuffer wrapper = ByteBuffer.wrap(bytearray, i+7, 4);
			int cfid = wrapper.getInt();
			
			
			System.out.println(year+ " "+ month+" "+date+" "+hour+" "+minute+" "+second+ " "+cf+ " "+ cfid);
		}
		String fileName = System.getProperty("user.dir")+"/outLog.txt";
				
		FileWriter writer = new FileWriter(fileName); 
		for(byte b: byteFile) {
		  writer.write(b);
		}
		writer.write(0xff);
		writer.close();
	}

}