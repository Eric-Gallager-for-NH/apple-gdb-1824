/*
 *     Generated by class-dump 3.3.4 (64 bit).
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2011 by Steve Nygard.
 */

#import "DBGDataValue.h"

#import "PBXLSDataValueDelegate-Protocol.h"

@class DBGDataType, DBGDataValueFormat, DVTPointerArray, NSArray, NSMutableArray, PBXLSDataValue;

@interface DBGGDBDataValue : DBGDataValue <PBXLSDataValueDelegate>
{
    PBXLSDataValue *_pbxDataValue;
    DBGDataType *_staticType;
    DVTPointerArray *_childrenValuesProxy;
    id _childLoadBlock;
    DBGGDBDataValue *_parent;
    DBGDataValueFormat *_format;
}

+ (id)keyPathsForValuesAffectingChildValuesCountValid;
+ (id)keyPathsForValuesAffectingValueValidity;
+ (id)keyPathsForValuesAffectingDynamicTypeHasChanged;
+ (id)keyPathsForValuesAffectingSummaryHasChanged;
+ (id)keyPathsForValuesAffectingValueHasChanged;
+ (id)keyPathsForValuesAffectingInScope;
+ (id)keyPathsForValuesAffectingSummary;
+ (id)keyPathsForValuesAffectingValue;
+ (id)keyPathsForValuesAffectingExpressionPath;
@property(retain) DBGDataType *staticType; // @synthesize staticType=_staticType;
@property(copy) id childLoadBlock; // @synthesize childLoadBlock=_childLoadBlock;
@property(retain) DBGGDBDataValue *parent; // @synthesize parent=_parent;
@property(retain, nonatomic) PBXLSDataValue *pbxDataValue; // @synthesize pbxDataValue=_pbxDataValue;
- (void)setContainsFunctionExpressions:(BOOL)arg1;
- (BOOL)containsFunctionExpressions;
- (void)dynamicTypeChanged:(id)arg1;
- (void)baseTypeChanged:(id)arg1;
- (void)childReplacedAtIndex:(unsigned long long)arg1 oldValue:(id)arg2 newValue:(id)arg3;
- (void)_handleChildrenInsertedOrRemoved;
- (void)childrenRemoved;
- (void)childrenInserted;
- (void)childValidityStatusChanged:(int)arg1;
- (void)watch;
- (BOOL)childValuesCountValid;
- (id)objectInChildrenProxyValuesAtIndex:(unsigned long long)arg1;
- (unsigned long long)countOfChildrenProxyValues;
@property(retain) NSArray *childValues; // @dynamic childValues;
- (int)valueValidity;
- (BOOL)hasChildValues;
- (BOOL)dynamicTypeHasChanged;
- (BOOL)summaryHasChanged;
- (BOOL)valueHasChanged;
- (BOOL)inScope;
- (void)setFormat:(id)arg1;
- (id)format;
- (id)blockStartAddress;
- (id)summary;
- (void)setValue:(id)arg1;
- (id)value;
- (id)_typeForDescription:(id)arg1;
- (id)expressionPath;
- (id)name;
- (id)initWithPBXLSDataValue:(id)arg1 stackFrame:(id)arg2;

// Remaining properties
@property(readonly) NSMutableArray *mutableChildValues; // @dynamic mutableChildValues;

@end

