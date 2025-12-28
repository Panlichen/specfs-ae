#include "timestamp.h"
#include <stddef.h> // Required for NULL
#include <stdint.h> // Required for uint32_t, uint64_t

// A bitmask for the 2 bits in the extra field that extend the seconds timestamp.
#define EXTRA_SEC_MASK 0x3

/**
 * PRECONDITION: You are given a pointer to an inode structure and a time type.
 * POSTCONDITION: Set the current time to the inode structure based on the time type.
 * The time is split across the primary time field and the extra field
 * to store nanosecond precision and extend the seconds range.
 */
void set_current_time(struct inode* node, int type) {
    if (node == NULL) {
        return;
    }

    struct timespec current_ts;
    // Get the current wall-clock time with nanosecond precision.
    if (clock_gettime(CLOCK_REALTIME, &current_ts) != 0) {
        // If clock_gettime fails, we cannot set the time, so we return.
        return;
    }

    // Deconstruct the timespec into the two 32-bit inode fields.
    // The main time field stores the lower 32 bits of the seconds count.
    uint32_t time_val = (uint32_t)current_ts.tv_sec;

    // The extra field is a combination of two values:
    // 1. The higher-order bits of the seconds count (bits 32 and 33).
    //    These are stored in the lower 2 bits of the extra field.
    uint32_t sec_extra = (uint32_t)((current_ts.tv_sec >> 32) & EXTRA_SEC_MASK);

    // 2. The nanosecond count. This is stored in the upper 30 bits.
    uint32_t nsec_val = (uint32_t)current_ts.tv_nsec;
    
    // Pack the nanoseconds and extra seconds bits into a single 32-bit value.
    uint32_t extra_val = (nsec_val << 2) | sec_extra;

    switch (type) {
        case ACCESS_TIME:
            node->atime = time_val;
            node->atime_extra = extra_val;
            break;
        case MODIFICATION_TIME:
            node->mtime = time_val;
            node->mtime_extra = extra_val;
            break;
        case CHANGE_TIME:
            node->ctime = time_val;
            node->ctime_extra = extra_val;
            break;
        default:
            // If the type is invalid, do nothing.
            break;
    }
}

/**
 * PRECONDITION: You are given a pointer to an inode structure and a time type.
 * POSTCONDITION: Get the current time from the inode structure based on the time type.
 * Calculate the time using the original timestamp and the extra field.
 * Return the time in a timespec structure.
 */
struct timespec get_current_time(struct inode* node, int type) {
    struct timespec ts = {0, 0};

    if (node == NULL) {
        return ts;
    }

    uint32_t time_val;
    uint32_t extra_val;

    // Select the correct pair of time fields to read from the inode.
    switch (type) {
        case ACCESS_TIME:
            time_val = node->atime;
            extra_val = node->atime_extra;
            break;
        case MODIFICATION_TIME:
            time_val = node->mtime;
            extra_val = node->mtime_extra;
            break;
        case CHANGE_TIME:
            time_val = node->ctime;
            extra_val = node->ctime_extra;
            break;
        default:
            // Invalid type; return the initialized zeroed timespec.
            return ts;
    }

    // Reconstruct the full timestamp from the two 32-bit fields.
    // 1. Extract the 2 extra bits for seconds from the lower part of extra_val.
    uint64_t sec_extra = extra_val & EXTRA_SEC_MASK;
    
    // 2. Extract the nanoseconds from the upper 30 bits of extra_val.
    ts.tv_nsec = extra_val >> 2;

    // 3. Combine the main 32-bit seconds with the 2-bit extension to form the
    //    full seconds count. The cast to uint64_t ensures the shift is correct.
    ts.tv_sec = (sec_extra << 32) | time_val;

    return ts;
}