caddr_t program_break, program_break_end;

caddr_t sbrk(int incr) {
    if(program_break == 0 || program_break + incr >= program_break_end) {
        errno = ENOMEM;
        return (caddr_t)-1;
    }

    caddr_t prev_break = program_break;
    program_break += incr;
    return prev_break;
}