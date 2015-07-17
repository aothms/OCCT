// Created on: 2003-06-04
// Created by: Galina KULIKOVA
// Copyright (c) 2003-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#ifndef _StepDimTol_ModifiedGeometricTolerance_HeaderFile
#define _StepDimTol_ModifiedGeometricTolerance_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <StepDimTol_LimitCondition.hxx>
#include <StepDimTol_GeometricTolerance.hxx>
class TCollection_HAsciiString;
class StepBasic_MeasureWithUnit;
class StepRepr_ShapeAspect;


class StepDimTol_ModifiedGeometricTolerance;
DEFINE_STANDARD_HANDLE(StepDimTol_ModifiedGeometricTolerance, StepDimTol_GeometricTolerance)

//! Representation of STEP entity ModifiedGeometricTolerance
class StepDimTol_ModifiedGeometricTolerance : public StepDimTol_GeometricTolerance
{

public:

  
  //! Empty constructor
  Standard_EXPORT StepDimTol_ModifiedGeometricTolerance();
  
  //! Initialize all fields (own and inherited)
  Standard_EXPORT void Init (const Handle(TCollection_HAsciiString)& aGeometricTolerance_Name, const Handle(TCollection_HAsciiString)& aGeometricTolerance_Description, const Handle(StepBasic_MeasureWithUnit)& aGeometricTolerance_Magnitude, const Handle(StepRepr_ShapeAspect)& aGeometricTolerance_TolerancedShapeAspect, const StepDimTol_LimitCondition aModifier);
  
  //! Returns field Modifier
  Standard_EXPORT StepDimTol_LimitCondition Modifier() const;
  
  //! Set field Modifier
  Standard_EXPORT void SetModifier (const StepDimTol_LimitCondition Modifier);




  DEFINE_STANDARD_RTTI(StepDimTol_ModifiedGeometricTolerance,StepDimTol_GeometricTolerance)

protected:




private:


  StepDimTol_LimitCondition theModifier;


};







#endif // _StepDimTol_ModifiedGeometricTolerance_HeaderFile