/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import "NSObject.h"

@class NSDate, NSMutableDictionary;

@interface PBXLSLaunchSession : NSObject
{
    int _state;
    NSDate *_sessionStartDate;
    BOOL _keepStats;
    NSMutableDictionary *_fixedFiles;
}

- (void)fixedFile:(id)arg1;
- (void)cleanup;
- (int)terminationStatus;
- (void)stop;
- (void)launch;
- (id)launchSessionStartDate;
- (int)launchSessionState;
- (void)setLaunchSessionState:(int)arg1;
- (id)displayName;
- (BOOL)keepStats;
- (void)setKeepStats:(BOOL)arg1;
- (id)init;

@end

