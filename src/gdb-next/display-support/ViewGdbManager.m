/* ViewGdbManager.m created by rhagy on Thu 27-Apr-1995 -*- Objective-C -*- */
#import "ViewGdbManager.h"
#import "DisplayHooks.h"
#import <Foundation/Foundation.h>
#import <stdio.h>

static void view_display_window_hook (FILE *instream, char *prompt)
{
  if ((instream == NULL) || (prompt == NULL)) {
    ; /* ??? */
  }
  if ((++pool_num_times) >= POOL_RELEASE_MAX_TIMES) {
    [pool release];
    pool_num_times = 0;
    pool = [[NSAutoreleasePool alloc] init];
  }
}

GdbManager* make_view_gdb_manager(void)
{
  return [[ViewGdbManager alloc] init];
}


@implementation ViewGdbManager

- (void) dealloc
{
  [super dealloc];
}

- (int) establishConnection
{
  NSDistantObject *dP;

  dP = [NSConnection
	 rootProxyForConnectionWithRegisteredName: displayProviderConnectionName
	 host: displayProviderHostName];

  if (dP == nil) {
    return 0;
  }

  [dP setProtocolForProxy: @protocol(ViewDisplayProvider)];
  [super setDisplayProvider: dP];

  /* I do NOT think notification on connection death works because, for viewing,
   * we are NOT in a RunLoop. */

  return 1;
}

- (void) engageHookFunctions
{
  window_hook = view_display_window_hook;
#if 0
  print_frame_info_listing_hook = tell_displayer_display_lines;
#endif /* 0 */
}

- (void) disengageHookFunctions
{
  window_hook = NULL;
#if 0
  print_frame_info_listing_hook = NULL;
#endif /* 0 */
}

@end

/* EOF */
