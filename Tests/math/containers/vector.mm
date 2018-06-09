//
//  vector.mm
//  Wrapper for Vector class
//
//  Created by Carlos David on 09/06/2018.
//  Copyright © 2018 cdalvaro. All rights reserved.
//

#import "vector.h"

#import "../../../NumericalPDEs/math/containers/vector.hpp"


using namespace cda::math::containers;

@implementation VectorWrapper : NSObject

- (Boolean)comparisonBetweenTwoVectors {
    Vector<double> vector1(10, 2);
    Vector<double> vector2(vector1);
    
    Boolean vectorsAreEqual = vector1 == vector2;
    Boolean vectorsAreDifferent = vector1 != vector2;
    
    return vectorsAreEqual && !vectorsAreDifferent;
}

- (Boolean)sumOfTwoVectors {
    Vector<double> vector1(10, 3);
    Vector<double> vector2(10, 4);
    Vector<double> expected(10, 7);
    
    auto result = vector1 + vector2;
    
    return result == expected;
}

- (void)moveConstructorWithVectorSize:(NSInteger)size {
    Vector<double> newVector(Vector<double>(size, 1));
    newVector[0] = 1;
}

@end
