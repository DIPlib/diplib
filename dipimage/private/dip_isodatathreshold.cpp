#include "dip_matlab_interface.h"

#include "diplib/segmentation.h"

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
	try {

		DML_MIN_ARGS(2);
		DML_MAX_ARGS(3);

		dip::Image const in = dml::GetImage( prhs[ 0 ] );
		dip::Image const mask = dml::GetImage( prhs[1] );
		dip::uint nThresholds = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 1;
		dml::MatlabInterface mi;
		dip::Image out = mi.NewImage();

		dip::FloatArray thresholds = dip::IsodataThreshold( in, mask, out, nThresholds );

		plhs[ 0 ] = dml::GetArray( out );
		if (nlhs > 1) {
			plhs[ 1 ] = dml::GetArray( thresholds );
		}

	} DML_CATCH
}