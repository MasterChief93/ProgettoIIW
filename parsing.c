#include <stdio.h>
#include <wurfl.h>
#include <device.h>
#include <string.h>

void wurfl_interrogation() {
	const char* root = "wurfl.xml";
	//const char* patches[] = NULL;

	wurfl_t* wurfl = wurfl_init(root,NULL);
	const char* user_agent = "Mozilla/5.0 (Linux; U; Android 3.1; xx-xx; GT-P7310 Build/HMJ37) AppleWebKit/534.13 (KHTML, like Gecko) Version/4.0 Safari/534.13";

	device_t* device = wurfl_match(wurfl,user_agent);

	char** capabilities = device_capabilities(device,NULL);
	char** caps_ptr = capabilities;

	while(caps_ptr!=NULL && *caps_ptr!=NULL) {
		if (strcmp(*caps_ptr,"resolution_width") == 0 || strcmp(*caps_ptr,"resolution_height") == 0) {
			fprintf(stderr, "%s: %s,\n", *caps_ptr, *(caps_ptr + 1));
		}
		caps_ptr+=2;
	}
	return;
	//This function has to use the User-Agent string in order to find the correct image quality factor
	//It should return a string or a float value
}

void get_request() {
	wurfl_interrogation();
	//Writing of HTTP Response
}

int main() {
	wurfl_interrogation();
	return 0;
	//Writing of HTTP Response
}
