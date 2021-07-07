char* video_mem = (char*)0xb8000;

void kmain(void) {
    *video_mem = 'O';
    *(video_mem + 2) = 'K';

    while (1) {

    }
}