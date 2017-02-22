%SELECT   Selects pixels from one image or another depending on a condition
%
% SYNOPSIS:
%  image_out = select(img1,img2,mask)
%  image_out = select(imgA,imgB,img1,img2,selector)
%
%  IMG1 and IMG2 are the source images. IMAGE_OUT will equal IMG1 for those
%  samples where the condition is true, and will equal IMG2 otherwise.
%
%  MASK is a binary image, where MASK is set, the condition is true.
%
%  SELECTOR is a comparison operator that defines the condition: IMGA <SELECTOR> IMGB.
%  These are the strings available: "==", "!=", ">", "<", ">=", "<=". "~=" is a
%  MATLAB-specific alias for "!=".
%
%  All images are singleton-expanded to a common size.
%
% DIPlib:
%  This function calls the two forms of the DIPlib function dip::Select.
