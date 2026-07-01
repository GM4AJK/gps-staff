import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';

class AppHeaderBar extends StatelessWidget {
  const AppHeaderBar({super.key, required this.title, required this.onBack});
  final String title;
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
            child: Text('←',
                style: GoogleFonts.montserrat(
                    fontSize: 24, color: Colors.white, height: 1)),
          ),
          const SizedBox(width: 16),
          Text(title,
              style: GoogleFonts.montserrat(
                  fontSize: 20,
                  fontWeight: FontWeight.w700,
                  color: Colors.white)),
        ],
      ),
    );
  }
}
