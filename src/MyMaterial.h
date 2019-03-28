//==============================================================================
/*
    CPSC 599.86 / 601.86 - Computer Haptics
    Winter 2018, University of Calgary

    This class extends the cMaterial class in CHAI3D to include additional
    object material properties. You may find it useful to store properties
    you need when creating surface textures for your assignment.
*/
//==============================================================================

#ifndef MYMATERIAL_H
#define MYMATERIAL_H

#include "chai3d.h"

//------------------------------------------------------------------------------
struct MyMaterial;
typedef std::shared_ptr<MyMaterial> MyMaterialPtr;
//------------------------------------------------------------------------------

enum material_type
{
	procedural_bump,
	procedural_friction,
	image_mapped_texture
};

struct MyMaterial : public chai3d::cMaterial
{
public:

    //! Constructor of cMaterial.
    MyMaterial();

    //! Shared MyMaterial allocator.
    static MyMaterialPtr create() { return (std::make_shared<MyMaterial>()); }


    //--------------------------------------------------------------------------
    // [CPSC.86] CUSTOM MATERIAL PROPERTIES
    //--------------------------------------------------------------------------

    double m_myMaterialProperty;
	material_type type;
	chai3d::cTexture2dPtr normal;
	chai3d::cTexture2dPtr roughness;
	chai3d::cTexture2dPtr height;

	double staticFriction;
	double dynamicFriction;
	// K == 1 is rough, K == 2 is smooth
	double K;
};

//------------------------------------------------------------------------------
#endif
