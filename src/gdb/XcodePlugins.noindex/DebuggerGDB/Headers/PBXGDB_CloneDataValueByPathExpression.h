/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import <DebuggerGDB/PBXGDB_SetDataValueForExpressionSequence.h>

@class PBXGDB_VarObjDataValue2;

@interface PBXGDB_CloneDataValueByPathExpression : PBXGDB_SetDataValueForExpressionSequence
{
    PBXGDB_VarObjDataValue2 *_dvToClone;
}

- (void)execute;
- (void)didGetPathExpression;
- (id)dvToClone;
- (void)setDVToClone:(id)arg1;
- (void)_createDVFromPathExpression;

@end

