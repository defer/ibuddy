#include <stdio.h>
#include <usb.h>
#include <unistd.h>
#include <pthread.h>


char setup_msg[]={0x22,0x09,0x00,0x02,0x01,0x00,0x00,0x00};
char msg[8]={0x55,0x53,0x42,0x43,0x00,0x40,0x02};

void send_message (usb_dev_handle *handle, int h, int b, int g, int Rr, int u, int d, int l, int r) {
		int A = 0;
		A = h&0x1;
		A = A << 1 | b&0x1;
		A = A << 1 | g&0x1;
		A = A << 1 | Rr&0x1;
		A = A << 1 | u&0x1;
		A = A << 1 | d&0x1;
		A = A << 1 | l&0x1;
		A = A << 1 | r&0x1;
		msg[7] = A;
		usb_control_msg(handle, 0x21,0x09,0x02,0x01,msg,8,100);
}


void *cena (void *data) {
	usb_dev_handle *handle = (usb_dev_handle*)data;
	int ret =usb_control_msg(handle, 0x21,0x09,0x02,0x01,setup_msg,8,100);
	printf ("%d\n",ret);

	int i = 0, heart = 0;
	while (true) {
		heart = (heart+1)&0x1;
		send_message (handle, heart, 1, 0, 1-heart, 0, 0, 1, 0);
		usleep (100000);
		send_message (handle, heart, 1, 0, 1-heart, 0, 0, 0, 1);
		usleep (100000);

		send_message (handle, heart, 1, 0, 1-heart, 0, 1, 0, 0);
		usleep (100000);
		send_message (handle, heart, 1, 0, 1-heart, 1, 0, 0, 0);
		usleep (100000);
	}

}

int main () {

	usb_init ();
	usb_find_busses ();
	usb_find_devices ();

	struct usb_bus *buses = usb_get_busses();
	struct usb_bus *bus = NULL;

	if (buses == NULL) {
		fprintf (stderr, "No buses found\n");
	}

	for (bus = buses; bus; bus = bus->next) {
		struct usb_device *dev;
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == 0x1130 && dev->descriptor.idProduct==0x0001) {
				usb_dev_handle *handle = usb_open (dev);
				pthread_t thread;
				pthread_create (&thread, NULL, cena, (void*)handle);
			}
		}
	}
	while (true) { sleep(4); }

	return 0;
}
