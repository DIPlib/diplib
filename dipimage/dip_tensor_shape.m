classdef dip_tensor_shape < uint8
   % dip_tensor_shape   Encodes how to interpret the tensor dimension in a dip_image object.
   % See DIPlib documentation for dip::Tensor for more information.
   enumeration
      COL_VECTOR(0)
      ROW_VECTOR(1)
      COL_MAJOR_MATRIX(2)
      ROW_MAJOR_MATRIX(3)
      DIAGONAL_MATRIX(4)
      SYMMETRIC_MATRIX(5)
      UPPTRIANG_MATRIX(6)
      LOWTRIANG_MATRIX(7)
   end
end
