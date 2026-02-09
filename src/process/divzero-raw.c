/**
 * @file
 *
 * Trigger a CPU exception by trying to divide by zero.
 */

#pragma GCC diagnostic ignored "-Wdiv-by-zero"

int _start(void)
{
    /* To infinity, and beyond! */
    return 10 / 0;
}
