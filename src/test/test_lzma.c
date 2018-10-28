#include <lzma.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	lzma_stream strm = LZMA_STREAM_INIT;

	uint32_t preset = LZMA_PRESET_DEFAULT;
	lzma_check check = LZMA_CHECK_CRC64;
	lzma_ret ret = lzma_easy_encoder(&strm, preset, check);
	if (ret != LZMA_OK)
	{
		fprintf(stderr, "lzma_easy_encoder: ret: %d", ret);
		return ret;
	}

	char *input_buf = "helloworld";
	size_t input_buf_len = strlen(input_buf);
	strm.next_in = input_buf;
	strm.avail_in = input_buf_len;

	uint64_t output_buf_bound = lzma_stream_buffer_bound(input_buf_len);
	char *output_buf = malloc(output_buf_bound);

	strm.avail_out = output_buf_bound;
	strm.next_out = output_buf;

	lzma_action action = LZMA_RUN;
	ret = lzma_code(&strm, action);
	if (ret != LZMA_OK)
	{
		fprintf(stderr, "lzma_code: ret: %d", ret);
		return ret;
	}

	lzma_end(&strm);
	return 0;
}
