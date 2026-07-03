import 'dart:math';
import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';

enum SurveyState { ready, inProgress, complete }

class SurveyInScreen extends StatefulWidget {
  const SurveyInScreen({super.key});

  @override
  State<SurveyInScreen> createState() => _SurveyInScreenState();
}

class _SurveyInScreenState extends State<SurveyInScreen> {
  // Mock state — State B (in progress), as per spec
  final SurveyState _state = SurveyState.inProgress;

  // Mock data (would come from BLE SURVEY_IN_STATUS notify)
  final int _elapsedSec   = 263;  // 4:23
  final int _targetSec    = 300;  // 5:00
  final double _meanAccM  = 0.8;  // metres
  final double _targetAccM = 3.0; // metres
  final int _observations = 1245;

  String _formatTime(int s) =>
      '${s ~/ 60}:${(s % 60).toString().padLeft(2, '0')}';

  bool get _durationMet => _elapsedSec >= _targetSec;
  bool get _accuracyMet => _meanAccM <= _targetAccM;

  @override
  Widget build(BuildContext context) {
    final stateBadgeText = switch (_state) {
      SurveyState.ready      => 'Ready',
      SurveyState.inProgress => '● In Progress',
      SurveyState.complete   => '✓ Complete',
    };
    final stateBadgeBg = switch (_state) {
      SurveyState.ready      => const Color(0xFFF5F5F5),
      SurveyState.inProgress => const Color(0xFFFFF8E1),
      SurveyState.complete   => const Color(0xFFE8F5E9),
    };
    final stateBadgeFg = switch (_state) {
      SurveyState.ready      => const Color(0xFF888888),
      SurveyState.inProgress => const Color(0xFFE65100),
      SurveyState.complete   => const Color(0xFF1B5E20),
    };

    return Scaffold(
      backgroundColor: kLightBg,
      resizeToAvoidBottomInset: false,
      body: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          const StatusBar(),
          // Custom header with state badge
          Container(
            height: 52,
            color: kLightHeaderBg,
            padding: const EdgeInsets.symmetric(horizontal: 16),
            child: Row(
              children: [
                GestureDetector(
                  onTap: () => Navigator.pop(context),
                  behavior: HitTestBehavior.opaque,
                  child: SizedBox(
                    width: 110,
                    height: 52,
                    child: Center(
                      child: Text('← Back',
                          style: GoogleFonts.montserrat(
                              fontSize: 15, fontWeight: FontWeight.w600,
                              color: Colors.white, height: 1)),
                    ),
                  ),
                ),
                Expanded(
                  child: Center(
                    child: Text('Survey-in',
                        style: GoogleFonts.montserrat(
                            fontSize: 20, fontWeight: FontWeight.w700,
                            color: Colors.white)),
                  ),
                ),
                Container(
                  padding:
                      const EdgeInsets.symmetric(horizontal: 12, vertical: 3),
                  decoration: BoxDecoration(
                    color: stateBadgeBg,
                    borderRadius: BorderRadius.circular(12),
                  ),
                  child: Text(stateBadgeText,
                      style: GoogleFonts.montserrat(
                          fontSize: 13, fontWeight: FontWeight.w700,
                          color: stateBadgeFg)),
                ),
              ],
            ),
          ),
          Expanded(
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                _LeftPanel(
                  state: _state,
                  elapsedSec: _elapsedSec,
                  targetSec: _targetSec,
                  meanAccM: _meanAccM,
                  targetAccM: _targetAccM,
                  durationMet: _durationMet,
                  accuracyMet: _accuracyMet,
                  formatTime: _formatTime,
                ),
                Expanded(
                  child: _RightPanel(
                    state: _state,
                    meanAccM: _meanAccM,
                    targetAccM: _targetAccM,
                    accuracyMet: _accuracyMet,
                    targetSec: _targetSec,
                    elapsedSec: _elapsedSec,
                    observations: _observations,
                    formatTime: _formatTime,
                    onCancel: () => Navigator.pop(context),
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

// ── Left panel — progress ring ────────────────────────────────────────────────

class _LeftPanel extends StatelessWidget {
  const _LeftPanel({
    required this.state,
    required this.elapsedSec,
    required this.targetSec,
    required this.meanAccM,
    required this.targetAccM,
    required this.durationMet,
    required this.accuracyMet,
    required this.formatTime,
  });

  final SurveyState state;
  final int elapsedSec;
  final int targetSec;
  final double meanAccM;
  final double targetAccM;
  final bool durationMet;
  final bool accuracyMet;
  final String Function(int) formatTime;

  Color get _ringColor => state == SurveyState.complete
      ? const Color(0xFF43A047)
      : kBlueAccent;

  double get _progress =>
      (targetSec > 0) ? (elapsedSec / targetSec).clamp(0.0, 1.0) : 0.0;

  @override
  Widget build(BuildContext context) {
    return Container(
      width: 460,
      decoration: const BoxDecoration(
        color: kLightBg,
        border: Border(right: BorderSide(color: Color(0xFFE0E0E0))),
      ),
      padding: const EdgeInsets.symmetric(vertical: 12),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          // Progress ring with overlay text
          SizedBox(
            width: 280,
            height: 280,
            child: Stack(
              alignment: Alignment.center,
              children: [
                CustomPaint(
                  size: const Size(280, 280),
                  painter: _RingPainter(
                    progress: _progress,
                    color: _ringColor,
                  ),
                ),
                Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    Text(
                      formatTime(elapsedSec),
                      style: GoogleFonts.montserrat(
                          fontSize: 48, fontWeight: FontWeight.w700,
                          color: const Color(0xFF222222), height: 1.1),
                    ),
                    Text(
                      'of ${formatTime(targetSec)}',
                      style: GoogleFonts.montserrat(
                          fontSize: 16, color: const Color(0xFF999999)),
                    ),
                    const SizedBox(height: 4),
                    Text(
                      'ELAPSED',
                      style: GoogleFonts.montserrat(
                          fontSize: 11, fontWeight: FontWeight.w700,
                          color: const Color(0xFFBBBBBB),
                          letterSpacing: 1),
                    ),
                  ],
                ),
              ],
            ),
          ),
          const SizedBox(height: 16),
          // Condition chips
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              _CondChip(
                label: 'Duration',
                value: '${formatTime(elapsedSec)} / ${formatTime(targetSec)}',
                sub: durationMet
                    ? 'Duration met'
                    : '${formatTime(targetSec - elapsedSec)} remaining',
                met: durationMet,
                inProgress: !durationMet,
              ),
              const SizedBox(width: 12),
              _CondChip(
                label: 'Absolute Accuracy',
                value: '±${meanAccM.toStringAsFixed(1)} m',
                sub: accuracyMet
                    ? 'target ≤ ${targetAccM.toStringAsFixed(0)} m — met'
                    : 'target ≤ ${targetAccM.toStringAsFixed(0)} m',
                met: accuracyMet,
                inProgress: !accuracyMet,
              ),
            ],
          ),
        ],
      ),
    );
  }
}

class _RingPainter extends CustomPainter {
  const _RingPainter({required this.progress, required this.color});
  final double progress;
  final Color color;

  @override
  void paint(Canvas canvas, Size size) {
    final center = Offset(size.width / 2, size.height / 2);
    final radius = size.width / 2 - 14;
    const strokeW = 22.0;

    // Background track
    canvas.drawCircle(
      center,
      radius,
      Paint()
        ..color = const Color(0xFFE0E0E0)
        ..style = PaintingStyle.stroke
        ..strokeWidth = strokeW,
    );

    // Progress arc — clockwise from 12 o'clock
    if (progress > 0) {
      canvas.drawArc(
        Rect.fromCircle(center: center, radius: radius),
        -pi / 2,              // start at 12 o'clock
        progress * 2 * pi,   // sweep clockwise
        false,
        Paint()
          ..color = color
          ..style = PaintingStyle.stroke
          ..strokeWidth = strokeW
          ..strokeCap = StrokeCap.round,
      );
    }
  }

  @override
  bool shouldRepaint(_RingPainter old) =>
      old.progress != progress || old.color != color;
}

class _CondChip extends StatelessWidget {
  const _CondChip({
    required this.label,
    required this.value,
    required this.sub,
    required this.met,
    required this.inProgress,
  });

  final String label;
  final String value;
  final String sub;
  final bool met;
  final bool inProgress;

  ({Color border, Color bg, Color fg}) get _colors {
    if (met)        return (border: const Color(0xFF43A047), bg: const Color(0xFFE8F5E9), fg: const Color(0xFF1B5E20));
    if (inProgress) return (border: kBlueAccent, bg: const Color(0xFFE3F2FD), fg: const Color(0xFF1565C0));
    return           (border: const Color(0xFFBBBBBB), bg: const Color(0xFFF5F5F5), fg: const Color(0xFF888888));
  }

  @override
  Widget build(BuildContext context) {
    final c = _colors;
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
      decoration: BoxDecoration(
        color: c.bg,
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: c.border, width: 2),
      ),
      child: Column(
        children: [
          Text(
            label.toUpperCase(),
            style: GoogleFonts.montserrat(
                fontSize: 11, fontWeight: FontWeight.w700,
                letterSpacing: 0.5, color: c.fg),
          ),
          const SizedBox(height: 4),
          Text(value,
              style: GoogleFonts.montserrat(
                  fontSize: 16, fontWeight: FontWeight.w700, color: c.fg)),
          const SizedBox(height: 2),
          Text(sub,
              style: GoogleFonts.montserrat(fontSize: 11, color: c.fg)),
        ],
      ),
    );
  }
}

// ── Right panel — stats, settings, cancel ────────────────────────────────────

class _RightPanel extends StatelessWidget {
  const _RightPanel({
    required this.state,
    required this.meanAccM,
    required this.targetAccM,
    required this.accuracyMet,
    required this.targetSec,
    required this.elapsedSec,
    required this.observations,
    required this.formatTime,
    required this.onCancel,
  });

  final SurveyState state;
  final double meanAccM;
  final double targetAccM;
  final bool accuracyMet;
  final int targetSec;
  final int elapsedSec;
  final int observations;
  final String Function(int) formatTime;
  final VoidCallback onCancel;

  @override
  Widget build(BuildContext context) {
    final remainSec = (targetSec - elapsedSec).clamp(0, targetSec);

    return Padding(
      padding: const EdgeInsets.all(18),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // Accuracy highlight box
          Container(
            padding: const EdgeInsets.all(12),
            decoration: BoxDecoration(
              color: accuracyMet
                  ? const Color(0xFFE8F5E9)
                  : const Color(0xFFFFF8E1),
              borderRadius: BorderRadius.circular(8),
              border: Border.all(
                color: accuracyMet
                    ? const Color(0xFF43A047)
                    : const Color(0xFFE65100),
                width: 2,
              ),
            ),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  'Current Position Accuracy',
                  style: GoogleFonts.montserrat(
                      fontSize: 11, fontWeight: FontWeight.w700,
                      letterSpacing: 1,
                      color: accuracyMet
                          ? const Color(0xFF2E7D32)
                          : const Color(0xFFE65100)),
                ),
                const SizedBox(height: 6),
                Row(
                  children: [
                    Text(
                      '±${meanAccM.toStringAsFixed(1)} m',
                      style: GoogleFonts.montserrat(
                          fontSize: 36, fontWeight: FontWeight.w700,
                          color: accuracyMet
                              ? const Color(0xFF1B5E20)
                              : const Color(0xFFE65100)),
                    ),
                    const SizedBox(width: 16),
                    Text(
                      accuracyMet
                          ? '✓ Target met (≤ ${targetAccM.toStringAsFixed(0)} m)'
                          : 'Target: ≤ ${targetAccM.toStringAsFixed(0)} m',
                      style: GoogleFonts.montserrat(
                          fontSize: 14,
                          color: accuracyMet
                              ? const Color(0xFF4CAF50)
                              : const Color(0xFFE65100)),
                    ),
                  ],
                ),
              ],
            ),
          ),
          const SizedBox(height: 14),

          // Settings (locked during B/C)
          Row(
            children: [
              Text('Settings',
                  style: GoogleFonts.montserrat(
                      fontSize: 11, fontWeight: FontWeight.w700,
                      letterSpacing: 1, color: const Color(0xFF999999))),
              if (state != SurveyState.ready) ...[
                const SizedBox(width: 8),
                Text('locked while survey-in is active',
                    style: GoogleFonts.montserrat(
                        fontSize: 11, color: const Color(0xFFBBBBBB))),
              ],
            ],
          ),
          const SizedBox(height: 8),
          Row(
            children: [
              Expanded(child: _SettingBox(
                  key_: 'Duration target',
                  value: '${formatTime(targetSec)} (${targetSec}s)')),
              const SizedBox(width: 10),
              Expanded(child: _SettingBox(
                  key_: 'Absolute accuracy target',
                  value: '${(targetAccM * 100).toInt()} cm (${targetAccM.toStringAsFixed(1)} m)')),
              const SizedBox(width: 10),
              Expanded(child: _SettingBox(
                  key_: 'Relative accuracy',
                  value: 'Always cm',
                  sub: 'RTK carrier-phase',
                  highlight: true)),
            ],
          ),
          const SizedBox(height: 14),

          // Live stats
          Text('Live Statistics',
              style: GoogleFonts.montserrat(
                  fontSize: 11, fontWeight: FontWeight.w700,
                  letterSpacing: 1, color: const Color(0xFF999999))),
          const SizedBox(height: 8),
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
            decoration: BoxDecoration(
              color: const Color(0xFFF8F9FA),
              borderRadius: BorderRadius.circular(6),
              border: Border.all(color: const Color(0xFFE0E0E0)),
            ),
            child: Column(
              children: [
                _StatRow('Observations accumulated',
                    observations.toString().replaceAllMapped(
                        RegExp(r'(\d{1,3})(?=(\d{3})+(?!\d))'),
                        (m) => '${m[1]},')),
                _StatRow('Elapsed', formatTime(elapsedSec)),
                _StatRow('Est. time remaining',
                    remainSec > 0 ? '~${formatTime(remainSec)}' : 'Complete'),
              ],
            ),
          ),

          const Spacer(),

          // Cancel or Start Streaming button
          SizedBox(
            height: 56,
            child: ElevatedButton(
              onPressed: state == SurveyState.complete
                  ? onCancel
                  : () async {
                      final confirmed = await showDialog<bool>(
                        context: context,
                        builder: (ctx) => AlertDialog(
                          backgroundColor: kLightBg,
                          title: Text('Cancel Survey-in?',
                              style: GoogleFonts.montserrat(
                                  fontWeight: FontWeight.w700,
                                  color: kLightText)),
                          content: Text(
                              'Are you sure you want to cancel? All survey progress will be lost.',
                              style: GoogleFonts.montserrat(
                                  color: kLightTextMuted)),
                          actions: [
                            TextButton(
                              onPressed: () => Navigator.pop(ctx, false),
                              child: Text('Keep Going',
                                  style: GoogleFonts.montserrat(
                                      color: kLightTextMuted)),
                            ),
                            ElevatedButton(
                              onPressed: () => Navigator.pop(ctx, true),
                              style: ElevatedButton.styleFrom(
                                  backgroundColor: const Color(0xFFD32F2F),
                                  foregroundColor: Colors.white),
                              child: Text('Cancel Survey-in',
                                  style: GoogleFonts.montserrat(
                                      fontWeight: FontWeight.w700)),
                            ),
                          ],
                        ),
                      );
                      if (confirmed == true) onCancel();
                    },
              style: ElevatedButton.styleFrom(
                backgroundColor: state == SurveyState.complete
                    ? const Color(0xFF43A047)
                    : const Color(0xFFD32F2F),
                foregroundColor: Colors.white,
                elevation: 0,
                shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(8)),
              ),
              child: Text(
                state == SurveyState.complete
                    ? 'Begin RTCM Streaming →'
                    : '✕  Cancel Survey-in',
                style: GoogleFonts.montserrat(
                    fontSize: 18, fontWeight: FontWeight.w700),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _SettingBox extends StatelessWidget {
  const _SettingBox({
    required this.key_,
    required this.value,
    this.sub,
    this.highlight = false,
  });

  final String key_;
  final String value;
  final String? sub;
  final bool highlight;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
      decoration: BoxDecoration(
        color: highlight
            ? const Color(0xFFE8F5E9)
            : const Color(0xFFF5F5F5),
        borderRadius: BorderRadius.circular(6),
        border: Border.all(
          color: highlight
              ? const Color(0xFFA5D6A7)
              : const Color(0xFFE0E0E0),
        ),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(key_,
              style: GoogleFonts.montserrat(
                  fontSize: 13,
                  color: highlight
                      ? const Color(0xFF2E7D32)
                      : const Color(0xFF666666))),
          const SizedBox(height: 2),
          Text(value,
              style: GoogleFonts.montserrat(
                  fontSize: 16, fontWeight: FontWeight.w700,
                  color: highlight
                      ? const Color(0xFF1B5E20)
                      : const Color(0xFF333333))),
          if (sub != null)
            Text(sub!,
                style: GoogleFonts.montserrat(
                    fontSize: 12,
                    color: highlight
                        ? const Color(0xFF4CAF50)
                        : const Color(0xFFAAAAAA))),
        ],
      ),
    );
  }
}

class _StatRow extends StatelessWidget {
  const _StatRow(this.label, this.value);
  final String label;
  final String value;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 3),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label,
              style: GoogleFonts.montserrat(
                  fontSize: 14, color: const Color(0xFF666666))),
          Text(value,
              style: GoogleFonts.montserrat(
                  fontSize: 16, fontWeight: FontWeight.w700,
                  color: const Color(0xFF222222))),
        ],
      ),
    );
  }
}
