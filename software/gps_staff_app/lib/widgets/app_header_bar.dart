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
      child: Stack(
        alignment: Alignment.center,
        children: [
          Center(
            child: Text(title,
                style: GoogleFonts.montserrat(
                    fontSize: 20,
                    fontWeight: FontWeight.w700,
                    color: Colors.white)),
          ),
          Align(
            alignment: Alignment.centerLeft,
            child: GestureDetector(
              onTap: onBack,
              behavior: HitTestBehavior.opaque,
              child: Container(
                height: 52,
                padding: const EdgeInsets.symmetric(horizontal: 16),
                alignment: Alignment.center,
                child: Text('← Back',
                    style: GoogleFonts.montserrat(
                        fontSize: 15,
                        fontWeight: FontWeight.w600,
                        color: Colors.white,
                        height: 1)),
              ),
            ),
          ),
        ],
      ),
    );
  }
}
