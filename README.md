# TOPAS TPS Source extension

Add pencil beam scanning support for [TOPAS](https://github.com/OpenTOPAS/OpenTOPAS).

This is an OpenTOPAS particle-source extension. It reads a compact spot-plan CSV plus an energy-dependent machine beam model, then generates primaries with scanning-magnet steering and BiGaussian emittance. Treatment-plan vendor formats and DICOM stay outside this source.

## Parameters

```text
s:So/CarbonPBS/Type = "PencilBeamScanning"
s:So/CarbonPBS/Component = "PBSBeamFrame"
s:So/CarbonPBS/BeamParticle = "GenericIon(6,12,6)"

s:So/CarbonPBS/SpotPlanFile = "spots.csv"
s:So/CarbonPBS/BeamModelFile = "beam_model.csv"

d:So/CarbonPBS/VirtualScanningMagneticX = 6227.8 mm
d:So/CarbonPBS/VirtualScanningMagneticY = 7008.6 mm
d:So/CarbonPBS/VirtualSourceToIsocenterDistance = 450 mm

b:So/CarbonPBS/SkipZeroWeightSpots = "True"
s:So/CarbonPBS/WeightMode = "Histories"
u:So/CarbonPBS/HistoriesScale = 1.0
b:So/CarbonPBS/InterpolateBeamModel = "False"
i:So/CarbonPBS/FirstSpot = 0
i:So/CarbonPBS/LastSpot = -1
```

`NumberOfHistoriesInRun` is set from the selected spots. Do not use Time Feature vectors for per-spot energy or optics.

`WeightMode = Histories` means CSV `weight` is the number of primaries. Sequential delivery: spot 0 is fully generated, then spot 1, and so on.

`Ts/NumberOfThreads` should stay 1. Sequential spot state is not validated in MT.

`SpotCoordinateConvention` defaults to `TPS`. Do not put it in the script unless you override it with `ComponentLocal`.

## Geometry

Hang `PBSBeamFrame` on the IEC gantry. The source only needs `s:So/.../Component = "PBSBeamFrame"`. Gantry / couch live on `IEC_G` / `IEC_S`.

```text
s:Ge/IEC_F/Parent = "World"
s:Ge/IEC_F/Type   = "Group"
s:Ge/IEC_G/Parent = "IEC_F"
s:Ge/IEC_G/Type   = "Group"

s:Ge/PBSBeamFrame/Parent = "IEC_G"
s:Ge/PBSBeamFrame/Type   = "Group"
```

`PBSBeamFrame` sits at the isocenter. The source puts the source plane at `y = -VirtualSourceToIsocenterDistance` and aims every spot at the origin. Do not translate or rotate `PBSBeamFrame` to place the nozzle. Change `IEC_G` rotations for other gantry angles.

`VirtualSourceToIsocenterDistance` is the source-to-isocenter distance. `VirtualScanningMagneticX/Y` are the scanning-magnet virtual distances used for spot steering.

## Limitations (version 1)

- No DICOM / vendor plan parser
- No MU-to-particle calibration
- No delivery timing / interplay
- Sequential scheduling only; not MT-safe
- BiGaussian emittance only
