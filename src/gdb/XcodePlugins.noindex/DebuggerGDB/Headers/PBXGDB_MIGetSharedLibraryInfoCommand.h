/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import <DebuggerGDB/PBXGDB_MISetSharedLibraryBaseCommand.h>

@class PBXLSStackFrame;

@interface PBXGDB_MIGetSharedLibraryInfoCommand : PBXGDB_MISetSharedLibraryBaseCommand
{
    PBXLSStackFrame *_stackFrame;
}

@property(retain) PBXLSStackFrame *stackFrame; // @synthesize stackFrame=_stackFrame;
- (id)GDBCommandString;

@end

