#!/usr/bin/python

# Color converters
# rgb <--> xyz
# xyz <--> lab
# lab <--> lch
#
# Sources
# * http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_Lab.html
# * http://www.brucelindbloom.com/index.html?Eqn_Lab_to_LCH.html
# * https://de.wikipedia.org/wiki/Lab-Farbraum
# * https://www.easyrgb.com/en/math.php
# * https://www.easyrgb.com/en/convert.php#inputFORM


import math


def rgbToXyz(rgb_in):
  """
  Convert rgb (values 0 to 255) to xyz (values from 0 to 1)
  """
  rgb = [i / 255 for i in rgb_in]

  x = 0.4124564 * rgb[0] + 0.3575761 * rgb[1] + 0.1804375 * rgb[2]
  y = 0.2126729 * rgb[0] + 0.7151522 * rgb[1] + 0.0721750 * rgb[2]
  z = 0.0193339 * rgb[0] + 0.1191920 * rgb[1] + 0.9503041 * rgb[2]
  return [x, y, z]


def xyzToRgb(xyz):
  """
  Convert xyz (values from 0 to 1) to rgb (values from 0 to 255)
  """
  r = xyz[0] *  3.2406 + xyz[1] * -1.5372 + xyz[2] * -0.4986
  g = xyz[0] * -0.9689 + xyz[1] *  1.8758 + xyz[2] *  0.0415
  b = xyz[0] *  0.0557 + xyz[1] * -0.2040 + xyz[2] *  1.0570

  r *= 255
  g *= 255
  b *= 255
  return [round(r), round(g), round(b)]


def getBounds():
  """
  Get boundaries for small values for conversion
  of xyz <--> lab
  """
  epsilon = 216/24389
  kappa = 24389/27
  return [epsilon, kappa]


def xyz_fcn(rel_col):
  """
  Calculate result for the normalized input color
  (color/color_ref).
  """
  epsilon, kappa = getBounds()
  if rel_col > epsilon:
    return math.pow(rel_col, 1/3)
  return (kappa * rel_col + 16)/116


def xyz_fcn_inv(rel_col):
  epsilon, kappa = getBounds()
  col_pow = math.pow(rel_col, 3)
  if col_pow > epsilon:
    return col_pow
  return ((rel_col * 116) - 16) / kappa


def getRefXyz():
  """
  Reference white after D65 and viewing angle of 10 degrees (CIE 1964)
  """
  xn_10 = 94.881/100
  yn_10 = 100/100
  zn_10 = 107.304/100
  return (xn_10, yn_10, zn_10)


def xyzToLab(xyz):
  """
  Convert xyz (values from 0 to 1) to lab values.
  L, a, b in percent
  """

  xn_10, yn_10, zn_10 = getRefXyz()

  # Normalize values
  x_rel = xyz[0]/xn_10
  y_rel = xyz[1]/yn_10
  z_rel = xyz[2]/zn_10

  L = 116 * xyz_fcn(y_rel) - 16
  a = 500 * (xyz_fcn(x_rel) - xyz_fcn(y_rel))
  b = 200 * (xyz_fcn(y_rel) - xyz_fcn(z_rel))
  return [L, a, b]


def labToXyz(lab):
  """
  Convert lab (percent) to xyz (values from 0 to 1).
  """
  y_tmp = (lab[0] + 16) / 116
  x_tmp = lab[1] / 500 + y_tmp
  z_tmp = y_tmp - lab[2] / 200

  xn_10, yn_10, zn_10 = getRefXyz()

  x = xyz_fcn_inv(x_tmp) * xn_10
  y = xyz_fcn_inv(y_tmp) * yn_10
  z = xyz_fcn_inv(z_tmp) * zn_10
  return [x, y, z]


def labToLch(lab):
  """
  Convert lab (percent) to lch values.
  lch is a representation of lab in polar coordinates
  (l is equal in both systems), h is the angle in radians
  """
  L = lab[0]
  c = math.sqrt(math.pow(lab[1], 2) + math.pow(lab[2], 2))
  h = math.atan2(lab[2], lab[1])
  return [L, c, h]


def lchToLab(lch):
  """
  Convert lch (h in radians) to lab (percent)
  """
  L = lch[0]
  a = math.cos(lch[2]) * lch[1]
  b = math.sin(lch[2]) * lch[1]
  return [L, a, b]


def print_rgb(rgb):
  print(f"r: {rgb[0]} g: {rgb[1]} b: {rgb[2]}")


def print_xyz(xyz):
  print(f"x: {xyz[0]} y: {xyz[1]} z: {xyz[2]}")


def print_lab(lab):
  print(f"l: {lab[0]} a: {lab[1]} b: {lab[2]}")


def print_lch(lch):
  print(f"l: {lch[0]} c: {lch[1]} h: {lch[2]}")


def main():
  rgb = [255, 0, 0]
  print_rgb(rgb)
  xyz = rgbToXyz(rgb)
  print_rgb(xyzToRgb(xyz))
  print_xyz(xyz)
  lab = xyzToLab(xyz)
  print_xyz(labToXyz(lab))
  print_lab(lab)
  lch = labToLch(lab)
  print_lab(lchToLab(lch))
  print_lch(lch)

  print("\n============================\n")

  rgb = [0, 0, 255]
  print_rgb(rgb)
  xyz = rgbToXyz(rgb)
  print_rgb(xyzToRgb(xyz))
  print_xyz(xyz)
  lab = xyzToLab(xyz)
  print_xyz(labToXyz(lab))
  print_lab(lab)
  lch = labToLch(lab)
  print_lab(lchToLab(lch))
  print_lch(lch)


if __name__ == "__main__":
  main()
