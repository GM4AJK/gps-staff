import 'dart:io';
import 'dart:math' as math;
import 'dart:typed_data';

/// ETRS89 (lat°, lon°, h m) → OSGB36 easting/northing + ODN height.
class Ostn15Result {
  final double osgbEasting;
  final double osgbNorthing;
  final double odnHeight;

  const Ostn15Result({
    required this.osgbEasting,
    required this.osgbNorthing,
    required this.odnHeight,
  });
}

/// OSTN15/OSGM15 coordinate transform service.
///
/// Call [load] once at startup; it scans for the binary grid file at
/// GPS_Staff/ostn15_osgm15.bin on any accessible storage volume.
/// After that, [transform] converts ETRS89 to OSGB36/ODN in ~microseconds.
class Ostn15Service {
  static final Ostn15Service instance = Ostn15Service._();
  Ostn15Service._();

  // GRS80 ellipsoid
  static const double _a  = 6378137.000;
  static const double _f  = 1.0 / 298.257222101;
  static final  double _b  = _a * (1.0 - _f);

  // National Grid TM projection constants
  static const double _F0  = 0.9996012717;
  static final  double _phi0 = 49.0 * math.pi / 180.0;
  static final  double _lam0 = -2.0 * math.pi / 180.0;
  static const double _E0  = 400000.0;
  static const double _N0  = -100000.0;
  static final  double _e2  = (_a * _a - _b * _b) / (_a * _a);
  static final  double _np  = (_a - _b) / (_a + _b); // 'n' parameter

  static const int    _numCols = 701;
  static const int    _numRows = 1251;
  static const String _magic   = 'OSTN1500';
  static const String _sdFile  = 'GPS_Staff/ostn15_osgm15.bin';

  ByteData? _grid;
  bool _attempted = false;

  bool get isAvailable => _grid != null;

  /// Scan storage volumes for the grid binary and load it into memory.
  /// Returns true on success. Safe to call multiple times; only loads once.
  Future<bool> load() async {
    if (_attempted) return isAvailable;
    _attempted = true;

    final file = await _findFile();
    if (file == null) return false;

    try {
      final bytes = await file.readAsBytes();
      if (bytes.length < 16) return false;
      if (String.fromCharCodes(bytes.sublist(0, 8)) != _magic) return false;

      final bd   = bytes.buffer.asByteData();
      final cols = bd.getUint32(8,  Endian.little);
      final rows = bd.getUint32(12, Endian.little);
      if (cols != _numCols || rows != _numRows) return false;

      _grid = bd;
      return true;
    } catch (_) {
      return false;
    }
  }

  /// Convert ETRS89 (decimal degrees, ellipsoidal height metres) to
  /// OSGB36 easting/northing (metres) and ODN orthometric height (metres).
  /// Returns null if [load] has not succeeded or the point is outside
  /// the Great Britain coverage area.
  Ostn15Result? transform(double latDeg, double lonDeg, double heightM) {
    final grid = _grid;
    if (grid == null) return null;

    final (ep, np) = etrs89ToTm(latDeg, lonDeg);

    final ei = ep ~/ 1000;
    final ni = np ~/ 1000;
    if (ei < 0 || ei >= _numCols - 1 || ni < 0 || ni >= _numRows - 1) {
      return null;
    }

    final t = (ep - ei * 1000.0) / 1000.0;
    final u = (np - ni * 1000.0) / 1000.0;

    final sw = _node(grid, ei,     ni);
    final se = _node(grid, ei + 1, ni);
    final ne = _node(grid, ei + 1, ni + 1);
    final nw = _node(grid, ei,     ni + 1);

    double interp(int i) =>
        (1 - t) * (1 - u) * sw[i] +
        t       * (1 - u) * se[i] +
        t       *       u * ne[i] +
        (1 - t) *       u * nw[i];

    return Ostn15Result(
      osgbEasting:  ep      + interp(0),
      osgbNorthing: np      + interp(1),
      odnHeight:    heightM - interp(2),
    );
  }

  /// ETRS89 geographic (decimal degrees) → ETRS89 TM projected (metres).
  /// Exposed as static so it can be tested independently of the binary file.
  static (double, double) etrs89ToTm(double latDeg, double lonDeg) {
    final phi = latDeg * math.pi / 180.0;
    final lam = lonDeg * math.pi / 180.0;

    final s    = math.sin(phi);
    final c    = math.cos(phi);
    final t    = math.tan(phi);
    final nu   = _a * _F0 / math.sqrt(1.0 - _e2 * s * s);
    final rho  = _a * _F0 * (1.0 - _e2) / math.pow(1.0 - _e2 * s * s, 1.5);
    final eta2 = nu / rho - 1.0;

    final M  = _meridionalArc(phi);
    final dl = lam - _lam0;
    final c3 = c * c * c;
    final c5 = c3 * c * c;
    final t2 = t * t;
    final t4 = t2 * t2;

    final ep = _E0
        + nu    * c                                                                              * dl
        + nu / 6   * c3 * (nu / rho - t2)                                                      * math.pow(dl, 3)
        + nu / 120 * c5 * (5.0 - 18.0 * t2 + t4 + 14.0 * eta2 - 58.0 * t2 * eta2)            * math.pow(dl, 5);

    final np = _N0 + M
        + nu / 2   * s * c                * dl * dl
        + nu / 24  * s * c3 * (5.0 - t2 + 9.0 * eta2)                                          * math.pow(dl, 4)
        + nu / 720 * s * c5 * (61.0 - 58.0 * t2 + t4)                                          * math.pow(dl, 6);

    return (ep, np);
  }

  static double _meridionalArc(double phi) {
    final n  = _np;
    final n2 = n * n;
    final n3 = n2 * n;
    final dp = phi - _phi0;
    final sp = phi + _phi0;
    return _b * _F0 * (
          (1.0 + n + 5.0 / 4.0 * n2 + 5.0 / 4.0 * n3) * dp
        - (3.0 * n + 3.0 * n2 + 21.0 / 8.0 * n3)       * math.sin(dp)   * math.cos(sp)
        + (15.0 / 8.0 * n2 + 15.0 / 8.0 * n3)          * math.sin(2 * dp) * math.cos(2 * sp)
        - 35.0 / 24.0 * n3                               * math.sin(3 * dp) * math.cos(3 * sp)
    );
  }

  List<double> _node(ByteData grid, int ei, int ni) {
    final off = 16 + (ni * _numCols + ei) * 12;
    return [
      grid.getFloat32(off,      Endian.little).toDouble(),
      grid.getFloat32(off + 4,  Endian.little).toDouble(),
      grid.getFloat32(off + 8,  Endian.little).toDouble(),
    ];
  }

  Future<File?> _findFile() async {
    final candidates = <String>['/storage/emulated/0/$_sdFile'];

    final storageDir = Directory('/storage');
    if (await storageDir.exists()) {
      await for (final entity in storageDir.list()) {
        final name = entity.path.split('/').last;
        if (name != 'emulated' && name != 'self' && entity is Directory) {
          candidates.add('${entity.path}/$_sdFile');
        }
      }
    }

    for (final p in candidates) {
      final f = File(p);
      if (await f.exists()) return f;
    }
    return null;
  }
}
