import diplib as dip
import numpy as np

# Demonstration of the measurement functionality

# Read in an image, segment it, and label the objects
img = dip.ImageRead('../cermet.ics')
img.SetPixelSize( 1.2, "µm" )
lab = dip.Label(img < 120)

# Compute some features. `msr` is a dip.Measurement object
msr = dip.MeasurementTool.Measure(lab, img, ["Size", "Feret", "Mean"])
print(msr)  # prints the measurement data as a table
# Note how the measuremets are in µm

# Indexing
feature = msr["Mean"]  # values for one feature, opaque object
object = msr[10]       # values for one object, opaque object
print(feature[10])     # a list with the values for one feature and one object -- some features have multiple values!
print(object["Feret"]) # note that you can index objects and features in either order

feature = msr["Feret"]
print(feature.Values())  # shows that this view has 5 values (shows name and units for each)
feature.Subset(1, 3)     # here we pick the values at indices 1 through 3 (3 values starting at index 1)
print(feature.Values())
feature.Subset(1)        # here we pick the value at index 1 only (note that the indexing is with respect
print(feature.Values())  #    to the full feature, not the current subset)

# Interaction with NumPy
feature = np.asarray(feature)  # no copy is made!
feature[0,0] = 10              # changes `msr`
all_data = np.asarray(msr)     # again, no copy is made
object = np.asarray(object)    # any of the views can be used as NumPy arrays

# Selection
lab_set = (msr["Size"] > 200) & (msr["Feret"] <= 22)  # Note Python operator precedence!
# We've selected based on the Size feature and the first value of the Feret feature (use the Subset() method
#    to use other feature values)
newlab = lab_set.Apply(lab)    # here we create a label image that contains only the selected objects
dip.Show(newlab, "labels")

lab_set.Relabel()
newlab = lab_set.Apply(lab)    # same, but the remaining objects have consecutive labels starting at 1
dip.Show(newlab, "labels")
