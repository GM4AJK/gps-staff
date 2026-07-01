import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';

enum PositionState { none, surveyIn, monument, ntrip }

class BaseConfigScreen extends StatelessWidget {
  const BaseConfigScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: kLightBg,
      body: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          const StatusBar(),
          _HeaderBar(onBack: () => Navigator.pop(context)),
          _PositionBanner(
            state: PositionState.surveyIn,
            label: 'Survey-in  ±0.8 m',
            onTap: () {},
          ),
          const Expanded(child: _ModeCards()),
          const _ConfigStrip(),
        ],
      ),
    );
  }
}

// ── Header bar ────────────────────────────────────────────────────────────────

class _HeaderBar extends StatelessWidget {
  const _HeaderBar({required this.onBack});
  final VoidCallback onBack;

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 52,
      color: kLightHeaderBg,
      padding: const EdgeInsets.symmetric(horizontal: 16),
      child: Row(
        children: [
          GestureDetector(
            onTap: onBack,
            child: Text(
              '←',
              style: GoogleFonts.montserrat(
                  fontSize: 24, color: Colors.white, height: 1),
            ),
          ),
          const SizedBox(width: 16),
          Text(
            'Base Configuration',
            style: GoogleFonts.montserrat(
              fontSize: 20,
              fontWeight: FontWeight.w700,
              color: Colors.white,
            ),
          ),
        ],
      ),
    );
  }
}

// ── Position banner ───────────────────────────────────────────────────────────

class _PositionBanner extends StatelessWidget {
  const _PositionBanner({
    required this.state,
    required this.label,
    required this.onTap,
  });

  final PositionState state;
  final String label;
  final VoidCallback onTap;

  ({Color bg, Color fg}) get _badge => switch (state) {
        PositionState.none     => (bg: const Color(0xFFFFEBEE), fg: const Color(0xFFC62828)),
        PositionState.surveyIn => (bg: const Color(0xFFFFF8E1), fg: const Color(0xFFE65100)),
        PositionState.monument => (bg: const Color(0xFFE8F5E9), fg: const Color(0xFF1B5E20)),
        PositionState.ntrip    => (bg: const Color(0xFFE3F2FD), fg: const Color(0xFF0D47A1)),
      };

  @override
  Widget build(BuildContext context) {
    final badge = _badge;
    return GestureDetector(
      onTap: onTap,
      child: Container(
        height: 44,
        decoration: const BoxDecoration(
          color: kLightBg,
          border: Border(bottom: BorderSide(color: kLightBorder)),
        ),
        padding: const EdgeInsets.symmetric(horizontal: 16),
        child: Row(
          children: [
            Text(
              'Base Position:',
              style: GoogleFonts.montserrat(
                  fontSize: 14,
                  fontWeight: FontWeight.w700,
                  color: kLightTextMuted),
            ),
            const SizedBox(width: 12),
            Container(
              padding:
                  const EdgeInsets.symmetric(horizontal: 12, vertical: 4),
              decoration: BoxDecoration(
                color: badge.bg,
                borderRadius: BorderRadius.circular(20),
              ),
              child: Text(
                label,
                style: GoogleFonts.montserrat(
                  fontSize: 14,
                  fontWeight: FontWeight.w700,
                  color: badge.fg,
                ),
              ),
            ),
            const SizedBox(width: 4),
            Text(
              '· tap to change →',
              style: GoogleFonts.montserrat(
                  fontSize: 13, color: kLightTextMuted),
            ),
          ],
        ),
      ),
    );
  }
}

// ── Mode cards ────────────────────────────────────────────────────────────────

class _ModeCards extends StatelessWidget {
  const _ModeCards();

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.fromLTRB(8, 16, 8, 8),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Expanded(
            child: _ModeCard(
              title: 'Survey-in',
              titleColor: kSurveyInColor,
              description:
                  'Average GNSS measurements over time to lock the base '
                  'position, then stream RTCM corrections.',
              statusText:
                  'Last survey: Complete  ·  Accuracy: ±0.8 m  ·  2026-06-25',
              buttonLabel: 'Start Survey-in →',
              buttonColor: kBtnBlue,
              onPressed: () {},
            ),
          ),
          Container(width: 2, color: kLightBorder),
          Expanded(
            child: _ModeCard(
              title: 'RTCM Data',
              titleColor: kRtcmColor,
              description:
                  'Broadcast RTCM corrections to the Rover over the LoRa '
                  'radio link. Requires a completed survey-in or fixed position.',
              statusText:
                  'Status: Streaming  ·  Rovers: 1 connected  ·  Signal: −78 dBm',
              buttonLabel: 'View RTCM Data →',
              buttonColor: kBtnGreen,
              onPressed: () {},
            ),
          ),
          Container(width: 2, color: kLightBorder),
          Expanded(
            child: _ModeCard(
              title: 'Satellite View',
              titleColor: kSatColor,
              description:
                  'Sky plot, signal strength and fix quality from the Base '
                  'GNSS receiver. Check sky visibility before a survey-in.',
              statusText:
                  'Sats: 14 (GPS 9 · GLO 5)  ·  Fix: RTK Fixed  ·  PDOP: 1.2',
              buttonLabel: 'View Satellites →',
              buttonColor: kBtnPurple,
              onPressed: () {},
            ),
          ),
        ],
      ),
    );
  }
}

class _ModeCard extends StatelessWidget {
  const _ModeCard({
    required this.title,
    required this.titleColor,
    required this.description,
    required this.statusText,
    required this.buttonLabel,
    required this.buttonColor,
    required this.onPressed,
  });

  final String title;
  final Color titleColor;
  final String description;
  final String statusText;
  final String buttonLabel;
  final Color buttonColor;
  final VoidCallback onPressed;

  @override
  Widget build(BuildContext context) {
    return Container(
      margin: const EdgeInsets.symmetric(horizontal: 8),
      decoration: BoxDecoration(
        color: kLightBg,
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: kLightBorder, width: 2),
      ),
      padding: const EdgeInsets.all(24),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            title,
            style: GoogleFonts.montserrat(
              fontSize: 32,
              fontWeight: FontWeight.w700,
              color: titleColor,
              height: 1,
            ),
          ),
          const SizedBox(height: 8),
          Expanded(
            child: Text(
              description,
              style: GoogleFonts.montserrat(
                  fontSize: 16, color: kLightTextSub, height: 1.4),
            ),
          ),
          const SizedBox(height: 16),
          Container(
            width: double.infinity,
            padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 10),
            decoration: BoxDecoration(
              color: kLightStatusBg,
              borderRadius: BorderRadius.circular(6),
            ),
            child: Text(
              statusText,
              style: GoogleFonts.montserrat(
                  fontSize: 14, color: kLightTextMuted),
            ),
          ),
          const SizedBox(height: 20),
          SizedBox(
            width: double.infinity,
            height: 56,
            child: ElevatedButton(
              onPressed: onPressed,
              style: ElevatedButton.styleFrom(
                backgroundColor: buttonColor,
                foregroundColor: Colors.white,
                elevation: 0,
                shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(8)),
              ),
              child: Text(
                buttonLabel,
                style: GoogleFonts.montserrat(
                    fontSize: 20, fontWeight: FontWeight.w700),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

// ── Config strip ──────────────────────────────────────────────────────────────

class _ConfigStrip extends StatelessWidget {
  const _ConfigStrip();

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 72,
      decoration: const BoxDecoration(
        color: kLightConfigBg,
        border: Border(top: BorderSide(color: kLightBorder)),
      ),
      padding: const EdgeInsets.fromLTRB(14, 6, 14, 6),
      child: Row(
        children: [
          _ConfigButton(icon: Icons.wifi, label: 'WiFi Settings', onPressed: () {}),
          const SizedBox(width: 12),
          _ConfigButton(
              icon: Icons.cell_tower,
              label: 'LoRa Settings',
              sublabel: '(ToDo)',
              onPressed: null),
          const SizedBox(width: 12),
          _ConfigButton(
              icon: Icons.location_on, label: 'Fixed Position', onPressed: () {}),
        ],
      ),
    );
  }
}

class _ConfigButton extends StatelessWidget {
  const _ConfigButton({
    required this.icon,
    required this.label,
    this.sublabel,
    required this.onPressed,
  });

  final IconData icon;
  final String label;
  final String? sublabel;
  final VoidCallback? onPressed;

  @override
  Widget build(BuildContext context) {
    final color = onPressed != null ? kLightConfigText : kLightTextMuted;
    return Expanded(
      child: Material(
        color: kLightStatusBg,
        borderRadius: BorderRadius.circular(8),
        child: InkWell(
          onTap: onPressed,
          borderRadius: BorderRadius.circular(8),
          child: Container(
            decoration: BoxDecoration(
              border: Border.all(color: kLightBorder, width: 2),
              borderRadius: BorderRadius.circular(8),
            ),
            padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Icon(icon, size: 20, color: color),
                const SizedBox(height: 2),
                Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    Text(
                      label,
                      style: GoogleFonts.montserrat(
                        fontSize: 14,
                        fontWeight: FontWeight.w700,
                        color: color,
                      ),
                    ),
                    if (sublabel != null) ...[
                      const SizedBox(width: 6),
                      Text(
                        sublabel!,
                        style: GoogleFonts.montserrat(
                            fontSize: 11, color: kLightTextMuted),
                      ),
                    ],
                  ],
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
