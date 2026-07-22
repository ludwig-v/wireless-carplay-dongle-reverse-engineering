/*
 * Carlinkit PTP-compatible USB source/sink gadget function.
 *
 * The original module is a modified f_sourcesink implementation rather
 * than the Android MTP/PTP transport.  Keep the implementation shared with
 * f_sourcesink.c and select its PTP descriptors and registration here.
 */

#define PTP_SOURCE_SINK
#include "f_ptp_sourcesink.c"
