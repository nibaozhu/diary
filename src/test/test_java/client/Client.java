import java.io.*;
import java.net.*;
import java.lang.*;

public class Client extends Thread {
	public void run() { }
	public static void main(String args[]) {
		for (int i = 0; i < args.length; i++) {
			System.out.printf("args[%d] = %s\n", i, args[i]);
		}

		String host = "0.0.0.0";
		int port = 12340;

		try {
			Client c = new Client();
			c.run();

			Socket s = new Socket(host, port);
			System.out.println(s);

			boolean ka = s.getKeepAlive();
			System.out.println(ka);

			boolean ic = s.isConnected();
			System.out.println(ic);

			int ss = s.getSendBufferSize();
			System.out.println(ss);

			int rs = s.getReceiveBufferSize();
			System.out.println(rs);

			InputStream is = s.getInputStream();
			System.out.println(is);

			OutputStream os = s.getOutputStream();
			System.out.println(os);

			byte[] b = new byte[]{0,0,0,0, 0,0,0,3, 0,0,0,3 };
			os.write(b);

			is.read();

			long millis = 1000 * 5;
			System.out.println(millis);
			c.sleep(millis);
			System.out.println(millis);

			s.close();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

	}

}
