// Created on: 1996-12-13
// Created by: Jean-Pierre COMBE/Odile Olivier
// Copyright (c) 1996-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#ifndef _AIS_PlaneTrihedron_HeaderFile
#define _AIS_PlaneTrihedron_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TCollection_AsciiString.hxx>
#include <AIS_InteractiveObject.hxx>
#include <Standard_Real.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_Integer.hxx>
#include <PrsMgr_PresentationManager3d.hxx>
#include <SelectMgr_Selection.hxx>
#include <AIS_KindOfInteractive.hxx>
#include <Quantity_NameOfColor.hxx>
class Geom_Plane;
class AIS_InteractiveObject;
class AIS_Line;
class AIS_Point;
class Prs3d_Presentation;
class Prs3d_Projector;
class Geom_Transformation;
class Quantity_Color;
class TCollection_AsciiString;


class AIS_PlaneTrihedron;
DEFINE_STANDARD_HANDLE(AIS_PlaneTrihedron, AIS_InteractiveObject)

//! To construct a selectable 2d axis system in a 3d
//! drawing. This can be placed anywhere in the 3d
//! system, and provides a coordinate system for
//! drawing curves and shapes in a plane.
//! There are 3 selection modes:
//! -   mode 0   selection of the whole plane "trihedron"
//! -   mode 1   selection of the origin of the plane "trihedron"
//! -   mode 2   selection of the axes.
//! Warning
//! For the presentation of planes and trihedra, the
//! millimetre is default unit of length, and 100 the default
//! value for the representation of the axes. If you modify
//! these dimensions, you must temporarily recover the
//! Drawer object. From inside it, take the Aspects in
//! which   the values for length are stocked, for example,
//! PlaneAspect for planes and FirstAxisAspect for
//! trihedra. Change these values and recalculate the presentation.
class AIS_PlaneTrihedron : public AIS_InteractiveObject
{

public:

  
  //! Initializes the plane aPlane. The plane trihedron is
  //! constructed from this and an axis.
  Standard_EXPORT AIS_PlaneTrihedron(const Handle(Geom_Plane)& aPlane);
  
  //! Returns the component specified in SetComponent.
  Standard_EXPORT Handle(Geom_Plane) Component();
  
  //! Creates an instance of the component object aPlane.
  Standard_EXPORT void SetComponent (const Handle(Geom_Plane)& aPlane);
  
  //! Returns the "XAxis".
  Standard_EXPORT Handle(AIS_Line) XAxis() const;
  
  //! Returns the "YAxis".
  Standard_EXPORT Handle(AIS_Line) YAxis() const;
  
  //! Returns the point of origin of the plane trihedron.
  Standard_EXPORT Handle(AIS_Point) Position() const;
  
  //! Sets the length of the X and Y axes.
  Standard_EXPORT void SetLength (const Standard_Real theLength);
  
  //! Returns the length of X and Y axes.
  Standard_EXPORT Standard_Real GetLength() const;
  
  //! Returns true if the display mode selected, aMode, is valid.
  Standard_EXPORT Standard_Boolean AcceptDisplayMode (const Standard_Integer aMode) const Standard_OVERRIDE;
  
  //! computes the presentation according to a point of view
  //! given by <aProjector>.
  //! To be Used when the associated degenerated Presentations
  //! have been transformed by <aTrsf> which is not a Pure
  //! Translation. The HLR Prs can't be deducted automatically
  //! WARNING :<aTrsf> must be applied
  //! to the object to display before computation  !!!
  Standard_EXPORT virtual void Compute (const Handle(Prs3d_Projector)& aProjector, const Handle(Geom_Transformation)& aTrsf, const Handle(Prs3d_Presentation)& aPresentation) Standard_OVERRIDE;
  
    virtual Standard_Integer Signature() const Standard_OVERRIDE;
  
  //! Returns datum as the type of Interactive Object.
    virtual AIS_KindOfInteractive Type() const Standard_OVERRIDE;
  
  //! Allows you to provide settings for the color aColor.
  Standard_EXPORT void SetColor (const Quantity_NameOfColor aColor) Standard_OVERRIDE;
  
  Standard_EXPORT void SetColor (const Quantity_Color& aColor) Standard_OVERRIDE;
  
    void SetXLabel (const TCollection_AsciiString& aLabel);
  
    void SetYLabel (const TCollection_AsciiString& aLabel);




  DEFINE_STANDARD_RTTI(AIS_PlaneTrihedron,AIS_InteractiveObject)

protected:

  
  Standard_EXPORT virtual void Compute (const Handle(PrsMgr_PresentationManager3d)& aPresentationManager, const Handle(Prs3d_Presentation)& aPresentation, const Standard_Integer aMode = 0) Standard_OVERRIDE;



private:

  
  Standard_EXPORT void Compute (const Handle(Prs3d_Projector)& aProjector, const Handle(Prs3d_Presentation)& aPresentation) Standard_OVERRIDE;
  
  Standard_EXPORT void ComputeSelection (const Handle(SelectMgr_Selection)& aSelection, const Standard_Integer aMode);

  Handle(Geom_Plane) myPlane;
  Handle(AIS_InteractiveObject) myShapes[3];
  TCollection_AsciiString myXLabel;
  TCollection_AsciiString myYLabel;


};


#include <AIS_PlaneTrihedron.lxx>





#endif // _AIS_PlaneTrihedron_HeaderFile