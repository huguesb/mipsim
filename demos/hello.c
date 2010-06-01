
void stop();
void print_string(const char *s);

void _start()
{
    print_string("Hello world!\n");
    
    stop();
}

void print_string(const char *s)
{
    __asm__(
    "addiu $v0, $zero, 4\t\n"
    "syscall\t\n"
    );
}

void stop()
{
    __asm__("break");
}

