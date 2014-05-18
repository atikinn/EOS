#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "utils.h"
#include "process.h"
#include "fd.h"
#include "time.h"
#include "svc.h"

#define is_leap(y) ((y & 3) == 0 && ((y % 25) != 0 || (y & 15) == 0))
// Credit for this fast alrogithm: 
// http://stackoverflow.com/questions/3220163/how-to-find-leap-year-programatically-in-c

typedef struct {    /* structure for the month object */
    char *name;     /* name of the month */
    int days;       /* number of days in this month */
} month_t;          

typedef struct {    /* internal time structure */
    char *month;
    int day; 
    int year; 
    int hour; 
    int min; 
    uint64_t sec; 
    uint16_t usec;
} tm_t;

enum date_intervals {   /* constants for the date command */
    SEC_IN_MIN          = 60,
    MIN_IN_HOUR         = 60, 
    HOURS_IN_DAY        = 24, 
    DAYS_IN_YEAR        = 365, 
    DAYS_IN_LEAPYEAR    = 366, 
    EPOCH               = 1970,
    SEC_IN_DAY          = 86400,
    FEBRUARY            = 1,
    TIME_ZONE           = -5
};

static const month_t months[] = {  
    { "January",    31 },
    { "February",   28 },
    { "March",      31 }, 
    { "April",      30 },
    { "May",        31 },
    { "June",       30 },
    { "July",       31 },
    { "August",     31 },
    { "September",  30 },
    { "October",    31 },
    { "November",   30 },
    { "December",   31 },
    { NULL,         0 }
};

static void compute_date(tm_t *tm, struct tval now);
static int compute_year(int *day);
static int compute_month(int *day, int year);

/*
 * main procedure of the date command
 *
 * argc - number of arguments
 * argv - argument vector
 *
 * returns SUCCESS on success, errno code on error
 */
int 
cmd_date(int argc, const char *argv[]) {
    if (argc != 1) return EARGNUM;   /* wrong number of arguments */

    struct tval now;				/* number of seconds and miliseconds from EPOCH */
    svc_gettime(&now);       	/* get current timestamp */

    tm_t current;                   /* internal date representation */
    compute_date(&current, now);    /* fill the structure */

    char fbuf[256];
    sprintf(fbuf, "%s %d, %d %02d:%02d:%02llu.%d\n", current.month, 
            current.day, current.year, current.hour, current.min, current.sec, 
            current.usec);
    eputs(STDOUT, fbuf);

    return SUCCESS;
}

int
cmd_setdate(int argc, const char *argv[]) {
    if (argc != 2) return EARGNUM;   /* wrong number of arguments */
	
    unsigned long long ts;
    if (strtoull_wrap(argv[1], &ts, DEC_BASE) < 0)
        return EPARSE;

    svc_settime(&ts);

	return SUCCESS;
}

/*
 * populates tm time structure from now structure
 *
 * tm - internal time representation structure
 * now - current timestamp
 *
 * returns void
 */
static 
void 
compute_date(tm_t *tm, struct tval now) {
    tm->sec = now.tval_sec;

    tm->day = 1 + tm->sec / SEC_IN_DAY; /* 1 for Jan 1, 1970 */
    tm->sec %= SEC_IN_DAY;

    tm->year = compute_year(&tm->day);

    int mon = compute_month(&tm->day, tm->year);
    tm->month = months[mon].name;

    tm->min = tm->sec / SEC_IN_MIN;
    tm->sec %= SEC_IN_MIN;

    tm->hour = tm->min / MIN_IN_HOUR;
    tm->min %= MIN_IN_HOUR;

    tm->usec = now.tval_usec;
}

/*
 * computes number of years passed since unix epoch
 *
 * day - number of days passed
 *
 * returns number of years
 */
static 
int 
compute_year(int *day) {
    int init_year = EPOCH;
    while (*day >= DAYS_IN_YEAR) {
        *day -= is_leap(init_year) ? DAYS_IN_LEAPYEAR : DAYS_IN_YEAR;
        init_year++;
    } 
    return init_year; 
}

/*
 * computes current month's index and current day
 *
 * day - number of days passed
 * year - number of years passed
 *
 * month index
 */
static 
int 
compute_month(int *day, int year) {
    int leap_year = is_leap(year);
    int mon;
    for (mon = 0; months[mon].name; mon++) {
        int mon_days = (leap_year && mon == FEBRUARY) ? months[mon].days + 1 : months[mon].days;
        if (*day <= mon_days) break;
        *day -= mon_days;
    } 
    return mon;
}
