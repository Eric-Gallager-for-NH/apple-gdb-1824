/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import "PBXGDB_BaseDataValueSequence.h"

@interface PBXGDB_GetFrameSymbolsDataValueSequence : PBXGDB_BaseDataValueSequence
{
    BOOL triedLocals;
    BOOL triedStatics;
}

- (void)execute;
- (void)_fetchLocals;
- (void)_gotResultArgumentVarObjs:(id)arg1;
- (void)_gotFileStaticsLITE:(id)arg1;
- (void)_fetchFileStaticsLITE;
- (void)_gotVerifyFileStatics:(id)arg1;
- (void)_gotResultLocalVarObjs:(id)arg1;
- (void)didFinish;
- (BOOL)handleErrorForMICommand:(id)arg1;
- (double)timeOutPeriod;

@end
