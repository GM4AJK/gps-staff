import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';
import 'base_config_screen.dart';
import 'rover_config_screen.dart';
import 'operating_mode_screen.dart';
import 'about_screen.dart';

class HomeScreen extends StatelessWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: kBgHome,
      body: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          const StatusBar(),
          const _HeroSection(),
          Expanded(
            child: Padding(
              padding: const EdgeInsets.fromLTRB(40, 20, 40, 16),
              child: Row(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  Expanded(
                    child: _BaseCard(
                      onTap: () => Navigator.push(
                        context,
                        MaterialPageRoute(
                            builder: (_) => const BaseConfigScreen()),
                      ),
                    ),
                  ),
                  const SizedBox(width: 20),
                  Expanded(
                    child: _RoverCard(
                      onTap: () => Navigator.push(context,
                          MaterialPageRoute(
                              builder: (_) => const RoverConfigScreen())),
                    ),
                  ),
                ],
              ),
            ),
          ),
          const _VersionStrip(),
        ],
      ),
    );
  }
}

// ── Hero ──────────────────────────────────────────────────────────────────────

class _HeroSection extends StatelessWidget {
  const _HeroSection();

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 168,
      decoration: BoxDecoration(
        border: Border(
          bottom: BorderSide(color: Colors.white.withValues(alpha: 0.06)),
        ),
      ),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          CustomPaint(
            size: const Size(80, 68),
            painter: _StaffIconPainter(),
          ),
          const SizedBox(height: 6),
          Text(
            'GPS Staff',
            style: GoogleFonts.montserrat(
              fontSize: 36,
              fontWeight: FontWeight.w700,
              color: kTextPrimary,
              letterSpacing: 1.44,
              height: 1,
            ),
          ),
          const SizedBox(height: 4),
          Text(
            'RTK SURVEY INSTRUMENT',
            style: GoogleFonts.montserrat(
              fontSize: 13,
              color: kTextMuted,
              letterSpacing: 1.04,
            ),
          ),
        ],
      ),
    );
  }
}

class _StaffIconPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    Paint arcPaint(Color color) => Paint()
      ..color = color
      ..style = PaintingStyle.stroke
      ..strokeWidth = 3
      ..strokeCap = StrokeCap.round;

    // Signal arcs — draw the top semicircle of each: start=pi, sweep=+pi
    canvas.drawArc(const Rect.fromLTWH(4, 2, 72, 72),
        3.14159, 3.14159, false, arcPaint(const Color(0x592196F3)));
    canvas.drawArc(const Rect.fromLTWH(15, 15, 50, 50),
        3.14159, 3.14159, false, arcPaint(const Color(0x992196F3)));
    canvas.drawArc(const Rect.fromLTWH(26, 28, 28, 28),
        3.14159, 3.14159, false, arcPaint(const Color(0xFF2196F3)));

    // Antenna
    canvas.drawCircle(
      const Offset(40, 45),
      5,
      Paint()..color = const Color(0xFF2196F3),
    );

    // Staff pole
    canvas.drawLine(
      const Offset(40, 50),
      const Offset(40, 64),
      Paint()
        ..color = const Color(0xFF546E7A)
        ..strokeWidth = 3
        ..strokeCap = StrokeCap.round,
    );

    // Ground foot (inverted V)
    canvas.drawPath(
      Path()
        ..moveTo(33, 62)
        ..lineTo(40, 68)
        ..lineTo(47, 62),
      Paint()
        ..color = const Color(0xFF546E7A)
        ..strokeWidth = 2.5
        ..strokeCap = StrokeCap.round
        ..strokeJoin = StrokeJoin.round
        ..style = PaintingStyle.stroke,
    );
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

// ── Base card ─────────────────────────────────────────────────────────────────

class _BaseCard extends StatelessWidget {
  const _BaseCard({required this.onTap});
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        decoration: BoxDecoration(
          color: kBgCard,
          borderRadius: BorderRadius.circular(12),
          border: Border.all(color: Colors.white.withValues(alpha: 0.08)),
        ),
        child: Row(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            _LeftAccent(color: kBlueAccent),
            Expanded(
              child: Padding(
                padding: const EdgeInsets.fromLTRB(18, 20, 22, 18),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    _CardHeader(
                      iconBg: kBlueIconBg,
                      iconFg: kBlueIconFg,
                      icon: '⊙',
                      title: 'Base Station',
                      subtitle: 'Fixed position · RTCM broadcast',
                      trailing: const _ConnectionDot(connected: false),
                    ),
                    const SizedBox(height: 14),
                    Expanded(
                      child: _Bullets(
                        dotColor: kBlueAccent,
                        textColor: kTextMuted,
                        items: const [
                          'Survey-in position or enter a known monument',
                          'Broadcast RTCM corrections over GFSK radio',
                          'WiFi NTRIP gateway to OS Net correction service',
                        ],
                      ),
                    ),
                    const SizedBox(height: 16),
                    _ActionButton(label: 'Configure Base →', onPressed: onTap),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

// ── Rover card ────────────────────────────────────────────────────────────────

class _RoverCard extends StatelessWidget {
  const _RoverCard({required this.onTap});
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        decoration: BoxDecoration(
          color: kBgCard,
          borderRadius: BorderRadius.circular(12),
          border: Border.all(color: Colors.white.withValues(alpha: 0.08)),
        ),
        child: Row(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            _LeftAccent(color: kGreen),
            Expanded(
              child: Padding(
                padding: const EdgeInsets.fromLTRB(18, 20, 22, 18),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    _CardHeader(
                      iconBg: const Color(0xFF1B5E20),
                      iconFg: const Color(0xFFA5D6A7),
                      icon: '⊛',
                      title: 'Rover',
                      subtitle: 'Mobile receiver · RTK fix',
                      trailing: const _ConnectionDot(connected: false),
                    ),
                    const SizedBox(height: 14),
                    Expanded(
                      child: _Bullets(
                        dotColor: kGreen,
                        textColor: kTextMuted,
                        items: const [
                          'Receive RTCM corrections from Base over GFSK',
                          'Achieve RTK fixed solution — centimetre accuracy',
                          'Log survey points and export to standard formats',
                        ],
                      ),
                    ),
                    const SizedBox(height: 16),
                    _ActionButton(label: 'Configure Rover →', onPressed: onTap),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

// ── Shared sub-widgets ────────────────────────────────────────────────────────

class _LeftAccent extends StatelessWidget {
  const _LeftAccent({required this.color});
  final Color color;

  @override
  Widget build(BuildContext context) {
    return Container(
      width: 4,
      decoration: BoxDecoration(
        color: color,
        borderRadius: const BorderRadius.only(
          topLeft: Radius.circular(12),
          bottomLeft: Radius.circular(12),
        ),
      ),
    );
  }
}

class _CardHeader extends StatelessWidget {
  const _CardHeader({
    required this.iconBg,
    required this.iconFg,
    required this.icon,
    required this.title,
    this.titleColor = kTextPrimary,
    required this.subtitle,
    this.subtitleColor = kTextMuted,
    required this.trailing,
  });

  final Color iconBg;
  final Color iconFg;
  final String icon;
  final String title;
  final Color titleColor;
  final String subtitle;
  final Color subtitleColor;
  final Widget trailing;

  @override
  Widget build(BuildContext context) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        Container(
          width: 44,
          height: 44,
          decoration: BoxDecoration(
            color: iconBg,
            borderRadius: BorderRadius.circular(10),
          ),
          alignment: Alignment.center,
          child: Text(icon,
              style: TextStyle(fontSize: 22, color: iconFg)),
        ),
        const SizedBox(width: 14),
        Expanded(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(
                title,
                style: GoogleFonts.montserrat(
                  fontSize: 20,
                  fontWeight: FontWeight.w700,
                  color: titleColor,
                ),
              ),
              const SizedBox(height: 2),
              Text(
                subtitle,
                style: GoogleFonts.montserrat(
                    fontSize: 12, color: subtitleColor),
              ),
            ],
          ),
        ),
        const SizedBox(width: 14),
        trailing,
      ],
    );
  }
}

class _ConnectionDot extends StatelessWidget {
  const _ConnectionDot({required this.connected});
  final bool connected;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Container(
          width: 8,
          height: 8,
          decoration: BoxDecoration(
            color: connected ? kGreen : kTextDim,
            shape: BoxShape.circle,
          ),
        ),
        const SizedBox(width: 6),
        Text(
          connected ? 'Connected' : 'Not connected',
          style: GoogleFonts.montserrat(fontSize: 11, color: kTextMuted),
        ),
      ],
    );
  }
}

class _Bullets extends StatelessWidget {
  const _Bullets({
    required this.dotColor,
    required this.textColor,
    required this.items,
  });

  final Color dotColor;
  final Color textColor;
  final List<String> items;

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: items
          .map(
            (text) => Padding(
              padding: const EdgeInsets.only(bottom: 8),
              child: Row(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text('·',
                      style: TextStyle(
                          color: dotColor, fontSize: 16, height: 1.3)),
                  const SizedBox(width: 10),
                  Expanded(
                    child: Text(
                      text,
                      style: GoogleFonts.montserrat(
                          fontSize: 13, color: textColor, height: 1.3),
                    ),
                  ),
                ],
              ),
            ),
          )
          .toList(),
    );
  }
}

class _ActionButton extends StatelessWidget {
  const _ActionButton({required this.label, required this.onPressed});
  final String label;
  final VoidCallback onPressed;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: double.infinity,
      height: 48,
      child: ElevatedButton(
        onPressed: onPressed,
        style: ElevatedButton.styleFrom(
          backgroundColor: kBlueButton,
          foregroundColor: Colors.white,
          elevation: 0,
          shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(8)),
        ),
        child: Text(
          label,
          style: GoogleFonts.montserrat(
              fontSize: 15,
              fontWeight: FontWeight.w700,
              letterSpacing: 0.45),
        ),
      ),
    );
  }
}

class _DisabledButton extends StatelessWidget {
  const _DisabledButton({required this.label});
  final String label;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: double.infinity,
      height: 48,
      child: ElevatedButton(
        onPressed: null,
        style: ElevatedButton.styleFrom(
          disabledBackgroundColor: kBgCardDisEl,
          disabledForegroundColor: kTextDim,
          elevation: 0,
          shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(8)),
        ),
        child: Text(
          label,
          style: GoogleFonts.montserrat(
              fontSize: 15, fontWeight: FontWeight.w700),
        ),
      ),
    );
  }
}

// ── Version strip ─────────────────────────────────────────────────────────────

class _VersionStrip extends StatelessWidget {
  const _VersionStrip();

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 28,
      color: kBgStatus,
      padding: const EdgeInsets.symmetric(horizontal: 18),
      child: Row(
        children: [
          GestureDetector(
            onTap: () => Navigator.push(context,
                MaterialPageRoute(
                    builder: (_) => const OperatingModeScreen())),
            child: Text(
              'Mode 1 — Local RF  ›',
              style: GoogleFonts.montserrat(
                  fontSize: 10,
                  color: const Color(0xFF546E7A),
                  letterSpacing: 0.4),
            ),
          ),
          const Spacer(),
          GestureDetector(
            onTap: () => Navigator.push(context,
                MaterialPageRoute(builder: (_) => const AboutScreen())),
            child: Text(
              'GPS STAFF · PCB v1.0 · fw 0.1.0  ›',
              style: GoogleFonts.montserrat(
                  fontSize: 10, color: kTextVersion, letterSpacing: 0.6),
            ),
          ),
        ],
      ),
    );
  }
}
