class RegisterClient {
	public static void main(String[] args) {
		for (int i = 0; i < args.length; i++) {
			System.out.printf("args[%d] = '%s'\n", i, args[i]);
		}

		RegisterCenter.ServiceLookupRequest.Builder builder = 
			RegisterCenter.ServiceLookupRequest.newBuilder();

		builder.setRequestId(args[0]);
		builder.setServiceName(args[1]);

		byte[] serializedMessage = builder.build().toByteArray();
		System.out.printf("serializedMessage[%d] = '%s'\n", 
			serializedMessage.length, serializedMessage);

		try {
			RegisterCenter.ServiceLookupRequest serviceLookupRequest = 
				RegisterCenter.ServiceLookupRequest.parseFrom(serializedMessage);

			System.out.printf("serviceLookupRequest = '{%s, %s}'\n", 
				serviceLookupRequest.getRequestId(),
				serviceLookupRequest.getServiceName());
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
