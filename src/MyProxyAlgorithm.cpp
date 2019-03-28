//==============================================================================
/*
    CPSC 599.86 / 601.86 - Computer Haptics
    Winter 2018, University of Calgary

    This class extends the cAlgorithmFingerProxy class in CHAI3D that
    implements the god-object/finger-proxy haptic rendering algorithm.
    It allows us to modify or recompute the force that is ultimately sent
    to the haptic device.

    Your job for this assignment is to implement the updateForce() method
    in this class to support for two new effects: force shading and haptic
    textures. Methods for both are described in Ho et al. 1999.
*/
//==============================================================================

#include "MyProxyAlgorithm.h"
#include "MyMaterial.h"

using namespace chai3d;

//==============================================================================
/*!
    This method uses the information computed earlier in
    computeNextBestProxyPosition() to calculate the force to be rendered.
    It first calls cAlgorithmFingerProxy::updateForce() to compute the base
    force from contact geometry and the constrained proxy position. That
    force can then be modified or recomputed in this function.

    Your implementation of haptic texture mapping will likely end up in this
    function. When this function is called, collision detection has already
    been performed, and the proxy point has already been updated based on the
    constraints found. Your job is to compute a force with all that information
    available to you.

    Useful variables to read:
        m_deviceGlobalPos   - current position of haptic device
        m_proxyGlboalPos    - computed position of the constrained proxy
        m_numCollisionEvents- the number of surfaces constraining the proxy
        m_collisionRecorderConstraint0,1,2
                            - up to three cCollisionRecorder structures with
                              cCollisionEvents that contain very useful
                              information about each contact

    Variables that this function should set/reset:
        
        m_lastGlobalForce   - this is what the operator ultimately feels!!!
*/
//==============================================================================

void MyProxyAlgorithm::updateForce()
{
    // get the base class to do basic force computation first
    cAlgorithmFingerProxy::updateForce();

    // TODO: compute force shading and texture forces here
	m_debugValue = m_numCollisionEvents;

    if (m_numCollisionEvents > 0)
    {
        // this is how you access collision information from the first constraint
        cCollisionEvent* c0 = &m_collisionRecorderConstraint0.m_nearestCollision;

		m_debugValue = c0->m_index;

        if (MyMaterialPtr material = std::dynamic_pointer_cast<MyMaterial>(c0->m_object->m_material))
        {
			//material->setd
			switch (material->type)
			{
				case material_type::procedural_bump :  
				{
					cVector3d uv = c0->m_triangles->getTexCoordAtPosition(c0->m_index, c0->m_localPos);
					// Ten bumps per one u-coordinate, so we multiply by 10*2pi
					m_lastGlobalForce = m_lastGlobalForce  + cVector3d(0.0, 0.0, 1.5*(1+ sin(uv.x()*10*2*M_PI)));
					break;
				}
				case material_type::procedural_friction:
				{
					cVector3d uv = c0->m_triangles->getTexCoordAtPosition(c0->m_index, c0->m_localPos);
					// five bumps per one u-coordinate, so we multiply by 5*2pi
					frictionModifier = (1.f - sin((uv.y() - 0.3) * 5 * 2 * M_PI));
					m_debugValue = frictionModifier;
					break;
				}
				case material_type::image_mapped_texture:
				{
					//store the force magnitude
					double mag = m_lastGlobalForce.length();


					//get uv of proxy point
					cVector3d uv = c0->m_triangles->getTexCoordAtPosition(c0->m_index, c0->m_localPos);
					double u = uv.x();
					double v = uv.y();
					//Wrap so 0 <= u, v <= 1
					u = u < 0 ? 1 + u : u;
					u = u > 1 ? u - 1 : u;
					v = v < 0 ? 1 + v : v;
					v = v > 1 ? v - 1 : v;
					
					//get pixel of proxy in normal map
					cColorf pixel;
					material->normal->m_image->getPixelColorInterpolated(1023.*u, 1023.*v, pixel);
					//Turn pixel into normal
					cVector3d n = cVector3d(pixel.getR() - 0.5, pixel.getG() - 0.5, pixel.getB() - 0.5);
					n.normalize();

					//Get unperturbed normal
					cVector3d N = computeShadedSurfaceNormal(c0);
					N.normalize();

					//compute matrix to rotate perturbed normal n
					double alpha = cAngle(n, N);
					m_debugValue = alpha;
					cVector3d cross = cCross(n, N);
					cross.normalize();
					//matrix to get perturbed surface normal w.r.t plane orientation
					cMatrix3d rot = cRotAxisAngleRad(cross.x(), cross.y(), cross.z(), alpha);

					//local gradient
					cVector3d gradH = rot*N;



					//M, final perturbed normal
					cVector3d M = N - gradH + cDot(gradH, N) * N;

					// get height from pixel
					cColorf height;
					material->height->m_image->getPixelColorInterpolated(1024.*u, 1024.*v, height);
					//h, the height of texture, times 0.01 as I will assume 0cm <= h <= 1cm
					double h = 0.01*height.getR();

					//d, penetration depth
					double d = (m_deviceGlobalPos - m_proxyGlobalPos).length() + h;

					//K, material value specified when creating material
					double K = material->K;

					//compute final force
					if (d >= K*h)
					{
						m_lastGlobalForce = ((d - K*h)*N + K*h*M);
						m_lastGlobalForce.normalize();
						m_lastGlobalForce *= mag;

					}
					else
					{
						m_lastGlobalForce = d*M;
						m_lastGlobalForce.normalize();
						m_lastGlobalForce *= mag;
					}
					

					//do friction stuff

					cColorf rough;
					material->roughness->m_image->getPixelColorInterpolated(1024.*u, 1024.*v, rough);
					frictionModifier = rough.getR();

					break;
				}

			}
        }
    }
}


//==============================================================================
/*!
    This method attempts to move the proxy, subject to friction constraints.
    This is called from computeNextBestProxyPosition() when the proxy is
    ready to move along a known surface.

    Your implementation of friction mapping will likely need to modify or
    replace the CHAI3D implementation in cAlgorithmFingerProxy. You may
    either copy the implementation from the base class and modify it to take
    into account a friction map, or use your own friction rendering from your
    previous assignment.

    The most important thing to do in this method is to write the desired
    proxy position into the m_nextBestProxyGlobalPos member variable.

    The input parameters to this function are as follows, all provided in the
    world (global) coordinate frame:

    \param  a_goal    The location to which we'd like to move the proxy.
    \param  a_proxy   The current position of the proxy.
    \param  a_normal  The surface normal at the obstructing surface.
    \param  a_parent  The surface along which we're moving.
*/
//==============================================================================
void MyProxyAlgorithm::testFrictionAndMoveProxy(const cVector3d& a_goal,
                                                const cVector3d& a_proxy,
                                                cVector3d &a_normal,
                                                cGenericObject* a_parent)

{
	//Copy paste chai3d code except that mud and mus are multiplied by our friction modifier

	// check if friction is enabled
	if (!a_parent->m_material->getUseHapticFriction())
	{
		m_nextBestProxyGlobalPos = a_goal;
		return;
	}

	// compute penetration depth; how far is the device "behind" the
	// plane of the obstructing surface
	cVector3d projectedGoal = cProjectPointOnPlane(m_deviceGlobalPos, a_proxy, a_normal);
	double penetrationDepth = cSub(m_deviceGlobalPos, projectedGoal).length();

	// find the appropriate friction coefficient
	double mud = a_parent->m_material->getDynamicFriction();
	double mus = a_parent->m_material->getStaticFriction();

	// THIS IS THE ONLY CHANGE TO THIS FUNCTION
	mud *= frictionModifier;
	mus *= frictionModifier;
	m_debugValue = mud;

	// no friction; don't try to compute friction cones
	if ((mud == 0) && (mus == 0))
	{
		m_nextBestProxyGlobalPos = a_goal;
		return;
	}

	// the corresponding friction cone radii
	double atmd = atan(mud);
	double atms = atan(mus);

	// compute a vector from the device to the proxy, for computing
	// the angle of the friction cone
	cVector3d vDeviceProxy = cSub(a_proxy, m_deviceGlobalPos);
	vDeviceProxy.normalize();

	// now compute the angle of the friction cone...
	double theta = acos(vDeviceProxy.dot(a_normal));

	// manage the "slip-friction" state machine

	// if the dynamic friction radius is for some reason larger than the
	// static friction radius, always slip
	if (mud > mus)
	{
		m_slipping = true;
	}

	// if we're slipping...
	else if (m_slipping)
	{
		if (theta < (atmd * m_frictionDynHysteresisMultiplier))
		{
			m_slipping = false;
		}
		else
		{
			m_slipping = true;
		}
	}

	// if we're not slipping...
	else
	{
		if (theta > atms)
		{
			m_slipping = true;
		}
		else
		{
			m_slipping = false;
		}
	}

	// the friction coefficient we're going to use...
	double mu;
	if (m_slipping)
	{
		mu = mud;
	}
	else
	{
		mu = mus;
	}

	// calculate the friction radius as the absolute value of the penetration
	// depth times the coefficient of friction
	double frictionRadius = fabs(penetrationDepth * mu);

	// calculate the distance between the proxy position and the current
	// goal position.
	double r = a_proxy.distance(a_goal);

	// if this distance is smaller than C_SMALL, we consider the proxy
	// to be at the same position as the goal, and we're done...
	if (r < C_SMALL)
	{
		m_nextBestProxyGlobalPos = a_proxy;
	}

	// if the proxy is outside the friction cone, update its position to
	// be on the perimeter of the friction cone...
	else if (r > frictionRadius)
	{
		m_nextBestProxyGlobalPos = cAdd(a_goal, cMul(frictionRadius / r, cSub(a_proxy, a_goal)));
	}

	// otherwise, if the proxy is inside the friction cone, the proxy
	// should not be moved (set next best position to current position)
	else
	{
		m_nextBestProxyGlobalPos = a_proxy;
	}

	// we're done; record the fact that we're still touching an object...
	return;
}
