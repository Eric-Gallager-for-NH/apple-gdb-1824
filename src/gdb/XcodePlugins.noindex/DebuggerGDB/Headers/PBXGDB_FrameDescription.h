/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import <DebuggerGDB/PBXGDB_BaseDescription.h>

@class NSString;

@interface PBXGDB_FrameDescription : PBXGDB_BaseDescription
{
    unsigned long long _address;
    NSString *_fileName;
    unsigned long long _lineNumber;
}

- (unsigned long long)lineNumber;
- (void)setLineNumber:(unsigned long long)arg1;
- (id)fileName;
- (void)setFileName:(id)arg1;
- (unsigned long long)address;
- (void)setAddress:(unsigned long long)arg1;
- (id)description;

@end

