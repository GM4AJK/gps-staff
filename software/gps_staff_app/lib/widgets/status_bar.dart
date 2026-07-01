import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';

class StatusBar extends StatelessWidget {
  const StatusBar({super.key});

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 28,
      color: kBgStatus,
      padding: const EdgeInsets.symmetric(horizontal: 14),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Row(
            children: [
              _BatteryChip(label: 'Base', percent: null),
              const SizedBox(width: 18),
              _BatteryChip(label: 'Rover', percent: null),
            ],
          ),
          Row(
            children: [
              Text(
                '----/--/-- --:--:--',
                style: GoogleFonts.robotoMono(fontSize: 12, color: kTextDim),
              ),
              const SizedBox(width: 14),
              _BatteryChip(label: null, percent: null),
            ],
          ),
        ],
      ),
    );
  }
}

class _BatteryChip extends StatelessWidget {
  const _BatteryChip({this.label, this.percent});
  final String? label;
  final int? percent;

  String _bar(int pct) {
    final filled = (pct / 25).round().clamp(0, 4);
    return '${'█' * filled}${'░' * (4 - filled)}';
  }

  @override
  Widget build(BuildContext context) {
    return RichText(
      text: TextSpan(
        style: GoogleFonts.robotoMono(fontSize: 12),
        children: [
          if (label != null)
            TextSpan(text: '$label ', style: const TextStyle(color: kTextMuted)),
          TextSpan(
            text: percent != null ? '▐${_bar(percent!)}▌ $percent%' : '—',
            style: TextStyle(
              color: percent != null ? Colors.green : kTextDim,
            ),
          ),
        ],
      ),
    );
  }
}
