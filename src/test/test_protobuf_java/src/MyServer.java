class MyServer {
	public static void main(String[] args) {
		for (int i = 0; i < args.length; i++) {
			System.out.printf("args[%d] = '%s'\n", i, args[i]);
		}

		RegisterCenter.ServiceRegisterRequest.Builder builder = 
			RegisterCenter.ServiceRegisterRequest.newBuilder();

		builder.setRequestId(args[0]);
		builder.setServiceName(args[1]);

		RegisterCenter.ServiceRegisterRequest serviceRegisterRequest = builder.build();

		byte[] serializedMessage = serviceRegisterRequest.toByteArray();
		System.out.printf("serializedMessage(%d): '%s'\n", 
			serializedMessage.length, serializedMessage);

		try {
			RegisterCenter.ServiceRegisterRequest serviceRegisterRequest2 = 
				RegisterCenter.ServiceRegisterRequest.parseFrom(serializedMessage);

			System.out.printf("deserialized serviceRegisterRequest2 {'%s', '%s'}\n", 
				serviceRegisterRequest2.getRequestId(), 
				serviceRegisterRequest2.getServiceName());
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

}
