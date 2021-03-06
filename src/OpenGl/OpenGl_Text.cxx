// Created on: 2011-07-13
// Created by: Sergey ZERCHANINOV
// Copyright (c) 2011-2013 OPEN CASCADE SAS
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

#include <OpenGl_AspectText.hxx>
#include <OpenGl_GlCore11.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OpenGl_ShaderManager.hxx>
#include <OpenGl_ShaderProgram.hxx>
#include <OpenGl_ShaderStates.hxx>
#include <OpenGl_Text.hxx>
#include <OpenGl_Workspace.hxx>
#include <OpenGl_View.hxx>
#include <OpenGl_VertexBufferCompat.hxx>

#include <Font_FontMgr.hxx>
#include <Font_FTFont.hxx>
#include <Graphic3d_TransformUtils.hxx>
#include <TCollection_HAsciiString.hxx>

namespace
{
  static const GLdouble THE_IDENTITY_MATRIX[16] =
  {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
  };

  //! Auxiliary tool for setting polygon offset temporarily.
  struct BackPolygonOffsetSentry
  {
    BackPolygonOffsetSentry (const Handle(OpenGl_Context)& theCtx)
    : myCtx (theCtx),
      myOffsetBack (!theCtx.IsNull() ? theCtx->PolygonOffset() : Graphic3d_PolygonOffset())
    {
      if (!theCtx.IsNull())
      {
        Graphic3d_PolygonOffset aPolyOffset = myOffsetBack;
        aPolyOffset.Mode = Aspect_POM_Fill;
        aPolyOffset.Units += 1.0f;
        theCtx->SetPolygonOffset (aPolyOffset);
      }
    }

    ~BackPolygonOffsetSentry()
    {
      if (!myCtx.IsNull())
      {
        myCtx->SetPolygonOffset (myOffsetBack);
      }
    }

  private:
    BackPolygonOffsetSentry (const BackPolygonOffsetSentry& );
    BackPolygonOffsetSentry& operator= (const BackPolygonOffsetSentry& );
  private:
    const Handle(OpenGl_Context)& myCtx;
    const Graphic3d_PolygonOffset myOffsetBack;
  };

} // anonymous namespace

// =======================================================================
// function : OpenGl_Text
// purpose  :
// =======================================================================
OpenGl_Text::OpenGl_Text()
: myWinX (0.0f),
  myWinY (0.0f),
  myWinZ (0.0f),
  myScaleHeight (1.0f),
  myPoint  (0.0f, 0.0f, 0.0f),
  myIs2d   (false),
  myHasPlane (false),
  myHasAnchorPoint (true)
{
  myParams.Height = 10;
  myParams.HAlign = Graphic3d_HTA_LEFT;
  myParams.VAlign = Graphic3d_VTA_BOTTOM;
}

// =======================================================================
// function : OpenGl_Text
// purpose  :
// =======================================================================
OpenGl_Text::OpenGl_Text (const Standard_Utf8Char* theText,
                          const OpenGl_Vec3&       thePoint,
                          const OpenGl_TextParam&  theParams)
: myWinX (0.0f),
  myWinY (0.0f),
  myWinZ (0.0f),
  myScaleHeight  (1.0f),
  myExportHeight (1.0f),
  myParams (theParams),
  myString (theText),
  myPoint  (thePoint),
  myIs2d   (false),
  myHasPlane (false),
  myHasAnchorPoint (true)
{
  //
}

// =======================================================================
// function : OpenGl_Text
// purpose  :
// =======================================================================
OpenGl_Text::OpenGl_Text (const Standard_Utf8Char* theText,
                          const gp_Ax2&            theOrientation,
                          const OpenGl_TextParam&  theParams,
                          const bool               theHasOwnAnchor)
: myWinX         (0.0),
  myWinY         (0.0),
  myWinZ         (0.0),
  myScaleHeight  (1.0),
  myExportHeight (1.0),
  myParams       (theParams),
  myString       (theText),
  myIs2d         (false),
  myOrientation  (theOrientation),
  myHasPlane     (true),
  myHasAnchorPoint (theHasOwnAnchor)
{
  const gp_Pnt& aPoint = theOrientation.Location();
  myPoint = OpenGl_Vec3 (static_cast<Standard_ShortReal> (aPoint.X()),
                         static_cast<Standard_ShortReal> (aPoint.Y()),
                         static_cast<Standard_ShortReal> (aPoint.Z()));
}

// =======================================================================
// function : SetPosition
// purpose  :
// =======================================================================
void OpenGl_Text::SetPosition (const OpenGl_Vec3& thePoint)
{
  myPoint = thePoint;
}

// =======================================================================
// function : SetFontSize
// purpose  :
// =======================================================================
void OpenGl_Text::SetFontSize (const Handle(OpenGl_Context)& theCtx,
                               const Standard_Integer        theFontSize)
{
  if (myParams.Height != theFontSize)
  {
    Release (theCtx.operator->());
  }
  myParams.Height = theFontSize;
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
void OpenGl_Text::Init (const Handle(OpenGl_Context)& theCtx,
                        const Standard_Utf8Char*      theText,
                        const OpenGl_Vec3&            thePoint)
{
  releaseVbos (theCtx.operator->());
  myIs2d   = false;
  myPoint  = thePoint;
  myString.FromUnicode (theText);
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
void OpenGl_Text::Init (const Handle(OpenGl_Context)& theCtx,
                        const Standard_Utf8Char*      theText,
                        const OpenGl_Vec3&            thePoint,
                        const OpenGl_TextParam&       theParams)
{
  if (myParams.Height != theParams.Height)
  {
    Release (theCtx.operator->());
  }
  else
  {
    releaseVbos (theCtx.operator->());
  }
  myIs2d   = false;
  myParams = theParams;
  myPoint  = thePoint;
  myString.FromUnicode (theText);
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
void OpenGl_Text::Init (const Handle(OpenGl_Context)&     theCtx,
                        const TCollection_ExtendedString& theText,
                        const OpenGl_Vec2&                thePoint,
                        const OpenGl_TextParam&           theParams)
{
  if (myParams.Height != theParams.Height)
  {
    Release (theCtx.operator->());
  }
  else
  {
    releaseVbos (theCtx.operator->());
  }
  myIs2d       = true;
  myParams     = theParams;
  myPoint.SetValues (thePoint, 0.0f);
  myString.FromUnicode (theText.ToExtString());
}

// =======================================================================
// function : ~OpenGl_Text
// purpose  :
// =======================================================================
OpenGl_Text::~OpenGl_Text()
{
  //
}

// =======================================================================
// function : releaseVbos
// purpose  :
// =======================================================================
void OpenGl_Text::releaseVbos (OpenGl_Context* theCtx)
{
  for (Standard_Integer anIter = 0; anIter < myVertsVbo.Length(); ++anIter)
  {
    Handle(OpenGl_VertexBuffer)& aVerts = myVertsVbo.ChangeValue (anIter);
    Handle(OpenGl_VertexBuffer)& aTCrds = myTCrdsVbo.ChangeValue (anIter);

    if (theCtx != NULL)
    {
      theCtx->DelayedRelease (aVerts);
      theCtx->DelayedRelease (aTCrds);
    }
    aVerts.Nullify();
    aTCrds.Nullify();
  }
  if (theCtx != NULL
  && !myBndVertsVbo.IsNull())
  {
    theCtx->DelayedRelease (myBndVertsVbo);
  }

  myTextures.Clear();
  myVertsVbo.Clear();
  myTCrdsVbo.Clear();
  myBndVertsVbo.Nullify();
}

// =======================================================================
// function : Release
// purpose  :
// =======================================================================
void OpenGl_Text::Release (OpenGl_Context* theCtx)
{
  releaseVbos (theCtx);
  if (!myFont.IsNull())
  {
    const TCollection_AsciiString aKey = myFont->ResourceKey();
    myFont.Nullify();
    if (theCtx != NULL)
    {
      theCtx->ReleaseResource (aKey, Standard_True);
    }
  }
}

// =======================================================================
// function : StringSize
// purpose  :
// =======================================================================
void OpenGl_Text::StringSize (const Handle(OpenGl_Context)& theCtx,
                              const NCollection_String&     theText,
                              const OpenGl_AspectText&      theTextAspect,
                              const OpenGl_TextParam&       theParams,
                              const unsigned int            theResolution,
                              Standard_ShortReal&           theWidth,
                              Standard_ShortReal&           theAscent,
                              Standard_ShortReal&           theDescent)
{
  theWidth   = 0.0f;
  theAscent  = 0.0f;
  theDescent = 0.0f;
  const TCollection_AsciiString aFontKey = FontKey (theTextAspect, theParams.Height, theResolution);
  Handle(OpenGl_Font) aFont = FindFont (theCtx, theTextAspect, theParams.Height, theResolution, aFontKey);
  if (aFont.IsNull() || !aFont->IsValid())
  {
    return;
  }

  theAscent  = aFont->Ascender();
  theDescent = aFont->Descender();

  GLfloat aWidth = 0.0f;
  for (NCollection_Utf8Iter anIter = theText.Iterator(); *anIter != 0;)
  {
    const Standard_Utf32Char aCharThis =   *anIter;
    const Standard_Utf32Char aCharNext = *++anIter;

    if (aCharThis == '\x0D' // CR  (carriage return)
     || aCharThis == '\a'   // BEL (alarm)
     || aCharThis == '\f'   // FF  (form feed) NP (new page)
     || aCharThis == '\b'   // BS  (backspace)
     || aCharThis == '\v')  // VT  (vertical tab)
    {
      continue; // skip unsupported carriage control codes
    }
    else if (aCharThis == '\x0A') // LF (line feed, new line)
    {
      theWidth = Max (theWidth, aWidth);
      aWidth   = 0.0f;
      continue;
    }
    else if (aCharThis == ' ')
    {
      aWidth += aFont->FTFont()->AdvanceX (aCharThis, aCharNext);
      continue;
    }
    else if (aCharThis == '\t')
    {
      aWidth += aFont->FTFont()->AdvanceX (' ', aCharNext) * 8.0f;
      continue;
    }

    aWidth += aFont->FTFont()->AdvanceX (aCharThis, aCharNext);
  }
  theWidth = Max (theWidth, aWidth);

  Handle(OpenGl_Context) aCtx = theCtx;
  aFont.Nullify();
  aCtx->ReleaseResource (aFontKey, Standard_True);
}

// =======================================================================
// function : Render
// purpose  :
// =======================================================================
void OpenGl_Text::Render (const Handle(OpenGl_Workspace)& theWorkspace) const
{
  theWorkspace->SetAspectFace (&theWorkspace->FontFaceAspect());
  theWorkspace->ApplyAspectFace();
  const OpenGl_AspectText*      aTextAspect  = theWorkspace->ApplyAspectText();
  const Handle(OpenGl_Context)& aCtx         = theWorkspace->GetGlContext();
  const Handle(OpenGl_TextureSet) aPrevTexture = aCtx->BindTextures (Handle(OpenGl_TextureSet)());

  // Bind custom shader program or generate default version
  aCtx->ShaderManager()->BindFontProgram (aTextAspect->ShaderProgramRes (aCtx));

  myOrientationMatrix = theWorkspace->View()->Camera()->OrientationMatrix();
  myProjMatrix.Convert (aCtx->ProjectionState.Current());

  // use highlight color or colors from aspect
  render (aCtx,
          *aTextAspect,
          theWorkspace->TextColor(),
          theWorkspace->TextSubtitleColor(),
          aCtx->Resolution());

  // restore aspects
  if (!aPrevTexture.IsNull())
  {
    aCtx->BindTextures (aPrevTexture);
  }

  // restore Z buffer settings
  if (theWorkspace->UseZBuffer())
  {
    glEnable (GL_DEPTH_TEST);
  }
}

// =======================================================================
// function : Render
// purpose  :
// =======================================================================
void OpenGl_Text::Render (const Handle(OpenGl_Context)& theCtx,
                          const OpenGl_AspectText&      theTextAspect,
                          const unsigned int            theResolution) const
{
  render (theCtx, theTextAspect,
          theTextAspect.Aspect()->ColorRGBA(),
          theTextAspect.Aspect()->ColorSubTitleRGBA(),
          theResolution);
}

// =======================================================================
// function : setupMatrix
// purpose  :
// =======================================================================
void OpenGl_Text::setupMatrix (const Handle(OpenGl_Context)& theCtx,
                               const OpenGl_AspectText&      theTextAspect,
                               const OpenGl_Vec3             theDVec) const
{
  OpenGl_Mat4d aModViewMat;
  OpenGl_Mat4d aProjectMat;
  if (myHasPlane && myHasAnchorPoint)
  {
    aProjectMat = myProjMatrix * myOrientationMatrix;
  }
  else
  {
    aProjectMat = myProjMatrix;
  }

  if (myIs2d)
  {
    Graphic3d_TransformUtils::Translate<GLdouble> (aModViewMat, myPoint.x() + theDVec.x(), myPoint.y() + theDVec.y(), 0.f);
    Graphic3d_TransformUtils::Scale<GLdouble> (aModViewMat, 1.f, -1.f, 1.f);
    Graphic3d_TransformUtils::Rotate<GLdouble> (aModViewMat, theTextAspect.Aspect()->GetTextAngle(), 0.f, 0.f, 1.f);
  }
  else
  {
    // align coordinates to the nearest integer
    // to avoid extra interpolation issues
    GLdouble anObjX, anObjY, anObjZ;
    Graphic3d_TransformUtils::UnProject<Standard_Real> (std::floor (myWinX + theDVec.x()),
                                                        std::floor (myWinY + theDVec.y()),
                                                        myWinZ + theDVec.z(),
                                                        OpenGl_Mat4d::Map (THE_IDENTITY_MATRIX),
                                                        OpenGl_Mat4d::Map (aProjectMat),
                                                        theCtx->Viewport(),
                                                        anObjX,
                                                        anObjY,
                                                        anObjZ);

    if (myHasPlane)
    {
      const gp_Dir& aVectorDir   = myOrientation.XDirection();
      const gp_Dir& aVectorUp    = myOrientation.Direction();
      const gp_Dir& aVectorRight = myOrientation.YDirection();

      aModViewMat.SetColumn (2, OpenGl_Vec3d (aVectorUp.X(), aVectorUp.Y(), aVectorUp.Z()));
      aModViewMat.SetColumn (1, OpenGl_Vec3d (aVectorRight.X(), aVectorRight.Y(), aVectorRight.Z()));
      aModViewMat.SetColumn (0, OpenGl_Vec3d (aVectorDir.X(), aVectorDir.Y(), aVectorDir.Z()));

      if (!myHasAnchorPoint)
      {
        OpenGl_Mat4d aPosMat;
        aPosMat.SetColumn (3, OpenGl_Vec3d (myPoint.x(), myPoint.y(), myPoint.z()));
        aPosMat *= aModViewMat;
        aModViewMat.SetColumn (3, aPosMat.GetColumn (3));
      }
      else
      {
        aModViewMat.SetColumn (3, OpenGl_Vec3d (anObjX, anObjY, anObjZ));
      }
    }
    else
    {
      Graphic3d_TransformUtils::Translate<GLdouble> (aModViewMat, anObjX, anObjY, anObjZ);
      Graphic3d_TransformUtils::Rotate<GLdouble> (aModViewMat, theTextAspect.Aspect()->GetTextAngle(), 0.0, 0.0, 1.0);
    }

    if (!theTextAspect.Aspect()->GetTextZoomable())
    {
      Graphic3d_TransformUtils::Scale<GLdouble> (aModViewMat, myScaleHeight, myScaleHeight, myScaleHeight);
    }
    else if (theCtx->HasRenderScale())
    {
      Graphic3d_TransformUtils::Scale<GLdouble> (aModViewMat, theCtx->RenderScaleInv(), theCtx->RenderScaleInv(), theCtx->RenderScaleInv());
    }
  }

  if (myHasPlane && !myHasAnchorPoint)
  {
    OpenGl_Mat4d aCurrentWorldViewMat;
    aCurrentWorldViewMat.Convert (theCtx->WorldViewState.Current());
    theCtx->WorldViewState.SetCurrent<Standard_Real> (aCurrentWorldViewMat * aModViewMat);
  }
  else
  {
    theCtx->WorldViewState.SetCurrent<Standard_Real> (aModViewMat);
  }
  theCtx->ApplyWorldViewMatrix();

  if (!myIs2d)
  {
    theCtx->ProjectionState.SetCurrent<Standard_Real> (aProjectMat);
    theCtx->ApplyProjectionMatrix();
  }

  // Upload updated state to shader program
  theCtx->ShaderManager()->PushState (theCtx->ActiveProgram());
}

// =======================================================================
// function : drawText
// purpose  :
// =======================================================================
void OpenGl_Text::drawText (const Handle(OpenGl_Context)& theCtx,
                            const OpenGl_AspectText&      theTextAspect) const
{
  (void )theTextAspect;
  if (myVertsVbo.Length() != myTextures.Length()
   || myTextures.IsEmpty())
  {
    return;
  }

  for (Standard_Integer anIter = 0; anIter < myTextures.Length(); ++anIter)
  {
    const GLuint aTexId = myTextures.Value (anIter);
    glBindTexture (GL_TEXTURE_2D, aTexId);

    const Handle(OpenGl_VertexBuffer)& aVerts = myVertsVbo.Value (anIter);
    const Handle(OpenGl_VertexBuffer)& aTCrds = myTCrdsVbo.Value (anIter);
    aVerts->BindAttribute (theCtx, Graphic3d_TOA_POS);
    aTCrds->BindAttribute (theCtx, Graphic3d_TOA_UV);

    glDrawArrays (GL_TRIANGLES, 0, GLsizei(aVerts->GetElemsNb()));

    aTCrds->UnbindAttribute (theCtx, Graphic3d_TOA_UV);
    aVerts->UnbindAttribute (theCtx, Graphic3d_TOA_POS);
  }
  glBindTexture (GL_TEXTURE_2D, 0);
}

// =======================================================================
// function : FontKey
// purpose  :
// =======================================================================
TCollection_AsciiString OpenGl_Text::FontKey (const OpenGl_AspectText& theAspect,
                                              const Standard_Integer   theHeight,
                                              const unsigned int       theResolution)
{
  const Font_FontAspect anAspect = theAspect.Aspect()->GetTextFontAspect() != Font_FA_Undefined
                                 ? theAspect.Aspect()->GetTextFontAspect()
                                 : Font_FA_Regular;
  return theAspect.Aspect()->Font()
       + TCollection_AsciiString(":") + Standard_Integer(anAspect)
       + TCollection_AsciiString(":") + Standard_Integer(theResolution)
       + TCollection_AsciiString(":") + theHeight;
}

// =======================================================================
// function : FindFont
// purpose  :
// =======================================================================
Handle(OpenGl_Font) OpenGl_Text::FindFont (const Handle(OpenGl_Context)& theCtx,
                                           const OpenGl_AspectText&      theAspect,
                                           const Standard_Integer        theHeight,
                                           const unsigned int            theResolution,
                                           const TCollection_AsciiString theKey)
{
  Handle(OpenGl_Font) aFont;
  if (theHeight < 2)
  {
    return aFont; // invalid parameters
  }

  if (!theCtx->GetResource (theKey, aFont))
  {
    Handle(Font_FontMgr) aFontMgr = Font_FontMgr::GetInstance();
    const Handle(TCollection_HAsciiString) aFontName = new TCollection_HAsciiString (theAspect.Aspect()->Font());
    const Font_FontAspect anAspect = theAspect.Aspect()->GetTextFontAspect() != Font_FA_Undefined
                                   ? theAspect.Aspect()->GetTextFontAspect()
                                   : Font_FA_Regular;
    Handle(Font_SystemFont) aRequestedFont = aFontMgr->FindFont (aFontName, anAspect, theHeight);
    Handle(Font_FTFont) aFontFt;
    if (!aRequestedFont.IsNull())
    {
      aFontFt = new Font_FTFont (Handle(Font_FTLibrary)());

      if (aFontFt->Init (aRequestedFont->FontPath()->ToCString(), theHeight, theResolution))
      {
        aFont = new OpenGl_Font (aFontFt, theKey);
        if (!aFont->Init (theCtx))
        {
          TCollection_ExtendedString aMsg;
          aMsg += "Font '";
          aMsg += theAspect.Aspect()->Font();
          aMsg += "' - initialization of GL resources has failed!";
          theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, aMsg);
          aFontFt.Nullify();
          aFont->Release (theCtx.operator->());
          aFont = new OpenGl_Font (aFontFt, theKey);
        }
      }
      else
      {
        TCollection_ExtendedString aMsg;
        aMsg += "Font '";
        aMsg += theAspect.Aspect()->Font();
        aMsg += "' is broken or has incompatible format! File path: ";
        aMsg += aRequestedFont->FontPath()->ToCString();
        theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, aMsg);
        aFontFt.Nullify();
        aFont = new OpenGl_Font (aFontFt, theKey);
      }
    }
    else
    {
      TCollection_ExtendedString aMsg;
      aMsg += "Font '";
      aMsg += theAspect.Aspect()->Font();
      aMsg += "' is not found in the system!";
      theCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, aMsg);
      aFont = new OpenGl_Font (aFontFt, theKey);
    }

    theCtx->ShareResource (theKey, aFont);
  }
  return aFont;
}

// =======================================================================
// function : drawRect
// purpose  :
// =======================================================================
void OpenGl_Text::drawRect (const Handle(OpenGl_Context)& theCtx,
                            const OpenGl_AspectText&      theTextAspect,
                            const OpenGl_Vec4&            theColorSubs) const
{
  Handle(OpenGl_ShaderProgram) aPrevProgram = theCtx->ActiveProgram();
  if (myBndVertsVbo.IsNull())
  {
    OpenGl_Vec2 aQuad[4] =
    {
      OpenGl_Vec2(myBndBox.Right, myBndBox.Bottom),
      OpenGl_Vec2(myBndBox.Right, myBndBox.Top),
      OpenGl_Vec2(myBndBox.Left,  myBndBox.Bottom),
      OpenGl_Vec2(myBndBox.Left,  myBndBox.Top)
    };
    if (theCtx->ToUseVbo())
    {
      myBndVertsVbo = new OpenGl_VertexBuffer();
    }
    else
    {
      myBndVertsVbo = new OpenGl_VertexBufferCompat();
    }
    myBndVertsVbo->Init (theCtx, 2, 4, aQuad[0].GetData());
  }

  // bind unlit program
  theCtx->ShaderManager()->BindFaceProgram (Handle(OpenGl_TextureSet)(), Graphic3d_TOSM_UNLIT,
                                            Graphic3d_AlphaMode_Opaque, Standard_False, Standard_False,
                                            Handle(OpenGl_ShaderProgram)());

#if !defined(GL_ES_VERSION_2_0)
  if (theCtx->core11 != NULL
   && theCtx->ActiveProgram().IsNull())
  {
    glBindTexture (GL_TEXTURE_2D, 0);
  }
#endif
  theCtx->SetColor4fv (theColorSubs);
  setupMatrix (theCtx, theTextAspect, OpenGl_Vec3 (0.0f, 0.0f, 0.0f));
  myBndVertsVbo->BindAttribute (theCtx, Graphic3d_TOA_POS);

  theCtx->core20fwd->glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

  myBndVertsVbo->UnbindAttribute (theCtx, Graphic3d_TOA_POS);
  theCtx->BindProgram (aPrevProgram);
}

// =======================================================================
// function : render
// purpose  :
// =======================================================================
void OpenGl_Text::render (const Handle(OpenGl_Context)& theCtx,
                          const OpenGl_AspectText&      theTextAspect,
                          const OpenGl_Vec4&            theColorText,
                          const OpenGl_Vec4&            theColorSubs,
                          const unsigned int            theResolution) const
{
  if (myString.IsEmpty())
  {
    return;
  }

  // Note that using difference resolution in different Views in same Viewer
  // will lead to performance regression (for example, text will be recreated every time).
  const TCollection_AsciiString aFontKey = FontKey (theTextAspect, myParams.Height, theResolution);
  if (!myFont.IsNull()
   && !myFont->ResourceKey().IsEqual (aFontKey))
  {
    // font changed
    const_cast<OpenGl_Text* > (this)->Release (theCtx.operator->());
  }

  if (myFont.IsNull())
  {
    myFont = FindFont (theCtx, theTextAspect, myParams.Height, theResolution, aFontKey);
  }
  if (!myFont->WasInitialized())
  {
    return;
  }

  if (myTextures.IsEmpty())
  {
    Font_TextFormatter aFormatter;
    aFormatter.SetupAlignment (myParams.HAlign, myParams.VAlign);
    aFormatter.Reset();

    aFormatter.Append (myString, *myFont->FTFont().operator->());
    aFormatter.Format();

    OpenGl_TextBuilder aBuilder;
    aBuilder.Perform (aFormatter,
                      theCtx,
                      *myFont.operator->(),
                      myTextures,
                      myVertsVbo,
                      myTCrdsVbo);

    aFormatter.BndBox (myBndBox);
    if (!myBndVertsVbo.IsNull())
    {
      myBndVertsVbo->Release (theCtx.get());
      myBndVertsVbo.Nullify();
    }
  }

  if (myTextures.IsEmpty())
  {
    return;
  }

  myExportHeight = 1.0f;
  myScaleHeight  = 1.0f;

  theCtx->WorldViewState.Push();
  myModelMatrix.Convert (theCtx->WorldViewState.Current() * theCtx->ModelWorldState.Current());

  const GLdouble aPointSize = (GLdouble )myFont->FTFont()->PointSize();
  if (!myIs2d)
  {
    Graphic3d_TransformUtils::Project<Standard_Real> (myPoint.x(), myPoint.y(), myPoint.z(),
                                                      myModelMatrix, myProjMatrix, theCtx->Viewport(),
                                                      myWinX, myWinY, myWinZ);

    // compute scale factor for constant text height
    if (theTextAspect.Aspect()->GetTextZoomable())
    {
      myExportHeight = aPointSize;
    }
    else
    {
      Graphic3d_Vec3d aPnt1, aPnt2;
      Graphic3d_TransformUtils::UnProject<Standard_Real> (myWinX, myWinY, myWinZ,
                                                          OpenGl_Mat4d::Map (THE_IDENTITY_MATRIX), myProjMatrix, theCtx->Viewport(),
                                                          aPnt1.x(), aPnt1.y(), aPnt1.z());
      Graphic3d_TransformUtils::UnProject<Standard_Real> (myWinX, myWinY + aPointSize, myWinZ,
                                                          OpenGl_Mat4d::Map (THE_IDENTITY_MATRIX), myProjMatrix, theCtx->Viewport(),
                                                          aPnt2.x(), aPnt2.y(), aPnt2.z());
      myScaleHeight = (aPnt2.y() - aPnt1.y()) / aPointSize;
    }
  }
  myExportHeight = aPointSize / myExportHeight;

#if !defined(GL_ES_VERSION_2_0)
  if (theCtx->core11 != NULL
   && theCtx->caps->ffpEnable)
  {
    glDisable (GL_LIGHTING);
  }

  const Standard_Integer aPrevPolygonMode  = theCtx->SetPolygonMode (GL_FILL);
  const bool             aPrevHatchingMode = theCtx->SetPolygonHatchEnabled (false);
#endif

  // setup depth test
  const bool hasDepthTest = !myIs2d
                         && theTextAspect.Aspect()->Style() != Aspect_TOST_ANNOTATION;
  if (!hasDepthTest)
  {
    glDisable (GL_DEPTH_TEST);
  }

  if (theCtx->core15fwd != NULL)
  {
    theCtx->core15fwd->glActiveTexture (GL_TEXTURE0);
  }
#if !defined(GL_ES_VERSION_2_0)
  // activate texture unit
  GLint aTexEnvParam = GL_REPLACE;
  if (theCtx->core11 != NULL)
  {
    glDisable (GL_TEXTURE_1D);
    glEnable  (GL_TEXTURE_2D);
    glGetTexEnviv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &aTexEnvParam);
    if (aTexEnvParam != GL_REPLACE)
    {
      glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }
  }
#endif

  // setup blending
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  const bool anAlphaToCoverageOld = theCtx->SetSampleAlphaToCoverage (false);

  // extra drawings
  switch (theTextAspect.Aspect()->DisplayType())
  {
    case Aspect_TODT_BLEND:
    {
    #if !defined(GL_ES_VERSION_2_0)
      glEnable  (GL_COLOR_LOGIC_OP);
      glLogicOp (GL_XOR);
    #endif
      break;
    }
    case Aspect_TODT_SUBTITLE:
    {
      BackPolygonOffsetSentry aPolygonOffsetTmp (hasDepthTest ? theCtx : Handle(OpenGl_Context)());
      drawRect (theCtx, theTextAspect, theColorSubs);
      break;
    }
    case Aspect_TODT_DEKALE:
    {
      BackPolygonOffsetSentry aPolygonOffsetTmp (hasDepthTest ? theCtx : Handle(OpenGl_Context)());
      theCtx->SetColor4fv (theColorSubs);
      setupMatrix (theCtx, theTextAspect, OpenGl_Vec3 (+1.0f, +1.0f, 0.0f));
      drawText    (theCtx, theTextAspect);
      setupMatrix (theCtx, theTextAspect, OpenGl_Vec3 (-1.0f, -1.0f, 0.0f));
      drawText    (theCtx, theTextAspect);
      setupMatrix (theCtx, theTextAspect, OpenGl_Vec3 (-1.0f, +1.0f, 0.0f));
      drawText    (theCtx, theTextAspect);
      setupMatrix (theCtx, theTextAspect, OpenGl_Vec3 (+1.0f, -1.0f, 0.0f));
      drawText    (theCtx, theTextAspect);
      break;
    }
    case Aspect_TODT_SHADOW:
    {
      BackPolygonOffsetSentry aPolygonOffsetTmp (hasDepthTest ? theCtx : Handle(OpenGl_Context)());
      theCtx->SetColor4fv (theColorSubs);
      setupMatrix (theCtx, theTextAspect, OpenGl_Vec3 (+1.0f, -1.0f, 0.0f));
      drawText    (theCtx, theTextAspect);
      break;
    }
    case Aspect_TODT_DIMENSION:
    case Aspect_TODT_NORMAL:
    {
      break;
    }
  }

  // main draw call
  theCtx->SetColor4fv (theColorText);
  setupMatrix (theCtx, theTextAspect, OpenGl_Vec3 (0.0f, 0.0f, 0.0f));
  drawText    (theCtx, theTextAspect);

  if (!myIs2d)
  {
    theCtx->ProjectionState.SetCurrent<Standard_Real> (myProjMatrix);
    theCtx->ApplyProjectionMatrix();
  }

#if !defined(GL_ES_VERSION_2_0)
  if (theCtx->core11 != NULL)
  {
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, aTexEnvParam);
  }
#endif

  if (theTextAspect.Aspect()->DisplayType() == Aspect_TODT_DIMENSION)
  {
    glDisable (GL_BLEND);
    if (!myIs2d)
    {
      glDisable (GL_DEPTH_TEST);
    }
  #if !defined(GL_ES_VERSION_2_0)
    if (theCtx->core11 != NULL)
    {
      glDisable (GL_TEXTURE_2D);
    }
  #endif
    const bool aColorMaskBack = theCtx->SetColorMask (false);

    glClear (GL_STENCIL_BUFFER_BIT);
    glEnable (GL_STENCIL_TEST);
    glStencilFunc (GL_ALWAYS, 1, 0xFF);
    glStencilOp (GL_KEEP, GL_KEEP, GL_REPLACE);

    drawRect (theCtx, theTextAspect, OpenGl_Vec4 (1.0f, 1.0f, 1.0f, 1.0f));

    glStencilFunc (GL_ALWAYS, 0, 0xFF);

    theCtx->SetColorMask (aColorMaskBack);
  }

  // reset OpenGL state
  glDisable (GL_BLEND);
  glDisable (GL_STENCIL_TEST);
#if !defined(GL_ES_VERSION_2_0)
  glDisable (GL_COLOR_LOGIC_OP);

  theCtx->SetPolygonMode         (aPrevPolygonMode);
  theCtx->SetPolygonHatchEnabled (aPrevHatchingMode);
#endif
  theCtx->SetSampleAlphaToCoverage (anAlphaToCoverageOld);

  // model view matrix was modified
  theCtx->WorldViewState.Pop();
  theCtx->ApplyModelViewMatrix();
}
