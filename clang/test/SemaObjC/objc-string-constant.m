// RUN: clang-cc %s -verify -fsyntax-only

#define nil 0       /* id of Nil instance */

@interface NSObject 
@end

@interface NSString : NSObject

@end

@interface NSMutableString : NSString

@end

@interface NSSimpleCString : NSString {
@protected
    char *bytes;
    int numBytes;
}
@end

@interface NSConstantString : NSSimpleCString
@end


@interface Subclass : NSObject 
- (NSString *)token;
@end

@implementation Subclass
- (NSString *)token;	// expected-warning {{semicolon at start of method definition is ignored}}
{
  NSMutableString *result = nil;

  return (result != nil) ? result : @"";
}
@end

