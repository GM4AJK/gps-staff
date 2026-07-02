import 'dart:math';
import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';
import '../widgets/app_header_bar.dart';

// ── Satellite data model ──────────────────────────────────────────────────────

class _Sat {
  const _Sat({
    required this.id,
    required this.gnssId, // 0=GPS, 6=GLONASS
    required this.az,
    required this.el,
    required this.cno,
    required this.used,
  });
  final String id;
  final int gnssId;
  final int az;   // azimuth degrees
  final int el;   // elevation degrees
  final int cno;  // carrier-to-noise, dBHz
  final bool used;
}

// Mock data — UK representative sky (GPS + GLONASS only)
const _mockSats = [
  _Sat(id: 'G01', gnssId: 0, az: 45,  el: 35, cno: 38, used: true),
  _Sat(id: 'G03', gnssId: 0, az: 120, el: 55, cno: 44, used: true),
  _Sat(id: 'G06', gnssId: 0, az: 200, el: 25, cno: 34, used: false),
  _Sat(id: 'G09', gnssId: 0, az: 310, el: 42, cno: 41, used: true),
  _Sat(id: 'G11', gnssId: 0, az: 165, el: 70, cno: 47, used: true),
  _Sat(id: 'G14', gnssId: 0, az: 240, el: 18, cno: 29, used: false),
  _Sat(id: 'G17', gnssId: 0, az: 80,  el: 48, cno: 43, used: true),
  _Sat(id: 'G19', gnssId: 0, az: 350, el: 32, cno: 37, used: true),
  _Sat(id: 'G22', gnssId: 0, az: 140, el: 62, cno: 45, used: true),
  _Sat(id: 'R02', gnssId: 6, az: 270, el: 38, cno: 36, used: true),
  _Sat(id: 'R07', gnssId: 6, az: 60,  el: 28, cno: 33, used: true),
  _Sat(id: 'R11', gnssId: 6, az: 190, el: 52, cno: 42, used: true),
  _Sat(id: 'R15', gnssId: 6, az: 330, el: 44, cno: 40, used: true),
  _Sat(id: 'R20', gnssId: 6, az: 100, el: 20, cno: 31, used: true),
];

// ── Screen ────────────────────────────────────────────────────────────────────

class SatelliteViewScreen extends StatelessWidget {
  const SatelliteViewScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: kLightBg,
      resizeToAvoidBottomInset: false,
      body: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          const StatusBar(),
          AppHeaderBar(
            title: 'Satellite View',
            onBack: () => Navigator.pop(context),
          ),
          Expanded(
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: const [
                _SkyPlotPanel(),
                Expanded(child: _RightPanel()),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

// ── Left panel — sky plot ─────────────────────────────────────────────────────

class _SkyPlotPanel extends StatelessWidget {
  const _SkyPlotPanel();

  @override
  Widget build(BuildContext context) {
    return Container(
      width: 480,
      decoration: const BoxDecoration(
        border: Border(right: BorderSide(color: Color(0xFFE0E0E0))),
      ),
      child: const CustomPaint(
        painter: _SkyPlotPainter(sats: _mockSats),
      ),
    );
  }
}

class _SkyPlotPainter extends CustomPainter {
  const _SkyPlotPainter({required this.sats});
  final List<_Sat> sats;

  static const _gpsColor  = Color(0xFF2196F3);
  static const _gloColor  = Color(0xFFE53935);
  static const _gpsDim    = Color(0xFF90CAF9);
  static const _gloDim    = Color(0xFFEF9A9A);

  @override
  void paint(Canvas canvas, Size size) {
    final cx = size.width / 2;
    final cy = size.height / 2;
    final radius = min(cx, cy) - 28; // leave room for N/E/S/W labels

    // Sky background
    canvas.drawCircle(
      Offset(cx, cy), radius,
      Paint()..color = const Color(0xFFF5F7FA),
    );
    canvas.drawCircle(
      Offset(cx, cy), radius,
      Paint()
        ..color = const Color(0xFFCCCCCC)
        ..style = PaintingStyle.stroke
        ..strokeWidth = 1.5,
    );

    // Elevation rings — dashed (30° and 60°)
    final dashedPaint = Paint()
      ..color = const Color(0xFFDDDDDD)
      ..style = PaintingStyle.stroke
      ..strokeWidth = 1.0;
    _drawDashedCircle(canvas, Offset(cx, cy), radius * 60 / 90, dashedPaint);
    _drawDashedCircle(canvas, Offset(cx, cy), radius * 30 / 90, dashedPaint);

    // Cardinal cross-hairs — dashed
    _drawDashedLine(canvas, Offset(cx, cy - radius), Offset(cx, cy + radius), dashedPaint);
    _drawDashedLine(canvas, Offset(cx - radius, cy), Offset(cx + radius, cy), dashedPaint);

    // Elevation ring labels (30°, 60°) — East side
    _drawText(canvas, '30°', Offset(cx + radius * 60 / 90 + 2, cy),
        fontSize: 9, color: const Color(0xFFBBBBBB), align: TextAlign.left);
    _drawText(canvas, '60°', Offset(cx + radius * 30 / 90 + 2, cy),
        fontSize: 9, color: const Color(0xFFBBBBBB), align: TextAlign.left);

    // Cardinal labels N/E/S/W
    _drawText(canvas, 'N', Offset(cx, cy - radius - 14),
        fontSize: 13, fontWeight: FontWeight.w700, color: const Color(0xFF555555));
    _drawText(canvas, 'S', Offset(cx, cy + radius + 14),
        fontSize: 13, fontWeight: FontWeight.w700, color: const Color(0xFF555555));
    _drawText(canvas, 'E', Offset(cx + radius + 14, cy),
        fontSize: 13, fontWeight: FontWeight.w700, color: const Color(0xFF555555));
    _drawText(canvas, 'W', Offset(cx - radius - 14, cy),
        fontSize: 13, fontWeight: FontWeight.w700, color: const Color(0xFF555555));

    // Satellite dots + labels
    for (final sat in sats) {
      final azRad = sat.az * pi / 180.0;
      final dist  = radius * (90 - sat.el) / 90.0;
      final px    = cx + dist * sin(azRad);
      final py    = cy - dist * cos(azRad);

      final baseColor = sat.gnssId == 0 ? _gpsColor : _gloColor;
      final dimColor  = sat.gnssId == 0 ? _gpsDim   : _gloDim;

      if (sat.used) {
        // Solid fill
        canvas.drawCircle(Offset(px, py), 9, Paint()..color = baseColor);
      } else {
        // Outline only
        canvas.drawCircle(Offset(px, py), 9,
            Paint()..color = kLightBg);
        canvas.drawCircle(Offset(px, py), 9,
            Paint()
              ..color = baseColor
              ..style = PaintingStyle.stroke
              ..strokeWidth = 2);
      }

      // PRN label below dot
      _drawText(canvas, sat.id, Offset(px, py + 12),
          fontSize: 9,
          fontWeight: sat.used ? FontWeight.w700 : FontWeight.w400,
          color: sat.used ? (sat.gnssId == 0 ? const Color(0xFF1565C0) : const Color(0xFFB71C1C)) : dimColor);
    }
  }

  // ── Helpers ────────────────────────────────────────────────────────────────

  void _drawDashedCircle(Canvas canvas, Offset center, double radius, Paint paint) {
    final path = Path()
      ..addOval(Rect.fromCircle(center: center, radius: radius));
    _drawDashedPath(canvas, path, paint);
  }

  void _drawDashedLine(Canvas canvas, Offset p1, Offset p2, Paint paint) {
    final path = Path()
      ..moveTo(p1.dx, p1.dy)
      ..lineTo(p2.dx, p2.dy);
    _drawDashedPath(canvas, path, paint);
  }

  void _drawDashedPath(Canvas canvas, Path path, Paint paint,
      {double dash = 5, double gap = 5}) {
    for (final metric in path.computeMetrics()) {
      double dist = 0;
      bool draw = true;
      while (dist < metric.length) {
        final len = draw ? dash : gap;
        if (draw) {
          canvas.drawPath(
            metric.extractPath(dist, (dist + len).clamp(0, metric.length)),
            paint,
          );
        }
        dist += len;
        draw = !draw;
      }
    }
  }

  void _drawText(
    Canvas canvas,
    String text,
    Offset position, {
    double fontSize = 12,
    FontWeight fontWeight = FontWeight.w400,
    Color color = const Color(0xFF555555),
    TextAlign align = TextAlign.center,
  }) {
    final tp = TextPainter(
      text: TextSpan(
        text: text,
        style: TextStyle(
          fontSize: fontSize,
          fontWeight: fontWeight,
          color: color,
          fontFamily: 'Montserrat',
        ),
      ),
      textDirection: TextDirection.ltr,
      textAlign: align,
    )..layout();

    final dx = align == TextAlign.center
        ? position.dx - tp.width / 2
        : position.dx;
    final dy = position.dy - tp.height / 2;
    tp.paint(canvas, Offset(dx, dy));
  }

  @override
  bool shouldRepaint(_SkyPlotPainter old) => false;
}

// ── Right panel — fix status + signal bars ────────────────────────────────────

class _RightPanel extends StatelessWidget {
  const _RightPanel();

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.fromLTRB(16, 16, 16, 12),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const _FixStatusGrid(),
          const SizedBox(height: 14),
          Text(
            'Signal Strength (CN0 dBHz)',
            style: GoogleFonts.montserrat(
              fontSize: 12, fontWeight: FontWeight.w700,
              letterSpacing: 1, color: const Color(0xFF888888)),
          ),
          const SizedBox(height: 6),
          const Expanded(child: _BarChart()),
          const SizedBox(height: 10),
          const _Legend(),
        ],
      ),
    );
  }
}

// ── Fix status grid ───────────────────────────────────────────────────────────

class _FixStatusGrid extends StatelessWidget {
  const _FixStatusGrid();

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
      decoration: BoxDecoration(
        color: const Color(0xFFF8F9FA),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: const Color(0xFFE0E0E0)),
      ),
      child: Row(
        children: [
          // Col 1: Fix + PDOP + Age
          Expanded(child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              _StatCell(label: 'Fix', widget: _FixBadge.rtkFixed()),
              const SizedBox(height: 8),
              const _StatCell(label: 'PDOP', value: '1.2'),
              const SizedBox(height: 8),
              const _StatCell(label: 'Age', value: '0.8 s'),
            ],
          )),
          const SizedBox(width: 16),
          // Col 2: Sats + HDOP + Time
          Expanded(child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const _StatCell(label: 'Sats used', value: '12', sub: '/ 14 tracked'),
              const SizedBox(height: 8),
              const _StatCell(label: 'HDOP', value: '0.8'),
              const SizedBox(height: 8),
              const _StatCell(label: 'Time', value: '10:42:31', sub: 'UTC'),
            ],
          )),
        ],
      ),
    );
  }
}

class _StatCell extends StatelessWidget {
  const _StatCell({required this.label, this.value, this.sub, this.widget});
  final String label;
  final String? value;
  final String? sub;
  final Widget? widget;

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(label,
            style: GoogleFonts.montserrat(
                fontSize: 11, fontWeight: FontWeight.w700,
                letterSpacing: 0.5, color: const Color(0xFF999999))),
        const SizedBox(height: 2),
        widget ?? Row(
          children: [
            Text(value ?? '',
                style: GoogleFonts.montserrat(
                    fontSize: 17, fontWeight: FontWeight.w700,
                    color: const Color(0xFF222222))),
            if (sub != null) ...[
              const SizedBox(width: 4),
              Text(sub!,
                  style: GoogleFonts.montserrat(
                      fontSize: 13, color: const Color(0xFF999999))),
            ],
          ],
        ),
      ],
    );
  }
}

class _FixBadge extends StatelessWidget {
  const _FixBadge({required this.text, required this.bg, required this.fg});
  const _FixBadge.rtkFixed()
      : text = 'RTK Fixed',
        bg = const Color(0xFFE8F5E9),
        fg = const Color(0xFF1B5E20);

  final String text;
  final Color bg;
  final Color fg;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 3),
      decoration: BoxDecoration(
        color: bg,
        borderRadius: BorderRadius.circular(12),
      ),
      child: Text(text,
          style: GoogleFonts.montserrat(
              fontSize: 13, fontWeight: FontWeight.w700, color: fg)),
    );
  }
}

// ── Signal bar chart ──────────────────────────────────────────────────────────

class _BarChart extends StatelessWidget {
  const _BarChart();

  static const _barW = 28.0;
  static const _gap  = 5.0;
  static const _maxCno = 50.0;
  static const _threshold = 35.0;

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(builder: (_, constraints) {
      final chartH = constraints.maxHeight - 18; // reserve 18px for PRN labels
      final thresholdY = chartH * (1 - _threshold / _maxCno);

      return Column(
        children: [
          Expanded(
            child: Stack(
              children: [
                // Bars
                Row(
                  crossAxisAlignment: CrossAxisAlignment.end,
                  children: [
                    for (final sat in _mockSats) ...[
                      Opacity(
                        opacity: sat.used ? 1.0 : 0.30,
                        child: Container(
                          width: _barW,
                          height: chartH * sat.cno / _maxCno,
                          decoration: BoxDecoration(
                            color: sat.gnssId == 0
                                ? const Color(0xFF2196F3)
                                : const Color(0xFFE53935),
                            borderRadius: const BorderRadius.vertical(
                                top: Radius.circular(3)),
                          ),
                        ),
                      ),
                      const SizedBox(width: _gap),
                    ],
                  ],
                ),
                // Threshold line
                Positioned(
                  top: thresholdY,
                  left: 0, right: 0,
                  child: Row(children: [
                    Expanded(
                      child: Container(
                        height: 1,
                        color: const Color(0xFFEF5350),
                      ),
                    ),
                  ]),
                ),
                // Threshold label
                Positioned(
                  top: thresholdY - 13,
                  right: 0,
                  child: Text('35 dBHz',
                      style: GoogleFonts.montserrat(
                          fontSize: 9, fontWeight: FontWeight.w700,
                          color: const Color(0xFFEF5350))),
                ),
              ],
            ),
          ),
          // PRN labels
          Row(
            children: [
              for (final sat in _mockSats) ...[
                SizedBox(
                  width: _barW,
                  child: Text(
                    sat.id,
                    textAlign: TextAlign.center,
                    style: GoogleFonts.montserrat(
                      fontSize: 8,
                      fontWeight: sat.used ? FontWeight.w700 : FontWeight.w400,
                      color: sat.used
                          ? (sat.gnssId == 0
                              ? const Color(0xFF1565C0)
                              : const Color(0xFFB71C1C))
                          : const Color(0xFFBDBDBD),
                    ),
                  ),
                ),
                const SizedBox(width: _gap),
              ],
            ],
          ),
        ],
      );
    });
  }
}

// ── Legend ────────────────────────────────────────────────────────────────────

class _Legend extends StatelessWidget {
  const _Legend();

  @override
  Widget build(BuildContext context) {
    return Wrap(
      spacing: 16,
      runSpacing: 4,
      children: [
        _LegendItem(dot: Container(
          width: 10, height: 10,
          decoration: const BoxDecoration(
              color: Color(0xFF2196F3), shape: BoxShape.circle),
        ), label: 'GPS (used)'),
        _LegendItem(dot: Container(
          width: 10, height: 10,
          decoration: BoxDecoration(
              shape: BoxShape.circle,
              border: Border.all(color: const Color(0xFF2196F3), width: 2)),
        ), label: 'GPS (not used)'),
        _LegendItem(dot: Container(
          width: 10, height: 10,
          decoration: const BoxDecoration(
              color: Color(0xFFE53935), shape: BoxShape.circle),
        ), label: 'GLONASS (used)'),
        _LegendItem(dot: SizedBox(
          width: 20, height: 10,
          child: CustomPaint(painter: _DashPainter()),
        ), label: '35 dBHz threshold', labelColor: const Color(0xFFEF5350)),
      ],
    );
  }
}

class _LegendItem extends StatelessWidget {
  const _LegendItem({required this.dot, required this.label, this.labelColor});
  final Widget dot;
  final String label;
  final Color? labelColor;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        dot,
        const SizedBox(width: 4),
        Text(label,
            style: GoogleFonts.montserrat(
                fontSize: 12,
                color: labelColor ?? const Color(0xFF555555))),
      ],
    );
  }
}

class _DashPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color = const Color(0xFFEF5350)
      ..strokeWidth = 1.5;
    double x = 0;
    while (x < size.width) {
      canvas.drawLine(Offset(x, size.height / 2),
          Offset(min(x + 4, size.width), size.height / 2), paint);
      x += 7;
    }
  }

  @override
  bool shouldRepaint(_DashPainter _) => false;
}
