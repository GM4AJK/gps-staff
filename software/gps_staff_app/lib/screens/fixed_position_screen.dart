import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';
import '../widgets/app_header_bar.dart';

enum _CoordSystem { osgb36, wgs84 }

class FixedPositionScreen extends StatefulWidget {
  const FixedPositionScreen({super.key});

  @override
  State<FixedPositionScreen> createState() => _FixedPositionScreenState();
}

class _FixedPositionScreenState extends State<FixedPositionScreen> {
  _CoordSystem _cs = _CoordSystem.osgb36;

  final _idCtrl       = TextEditingController(text: 'OS-BM-TP-1234');
  final _eastCtrl     = TextEditingController(text: '651409.903');
  final _northCtrl    = TextEditingController(text: '313177.270');
  final _heightOdnCtrl = TextEditingController(text: '24.700');
  final _latCtrl      = TextEditingController();
  final _lonCtrl      = TextEditingController();
  final _heightCtrl   = TextEditingController();

  @override
  void dispose() {
    for (final c in [_idCtrl, _eastCtrl, _northCtrl, _heightOdnCtrl,
                     _latCtrl, _lonCtrl, _heightCtrl]) {
      c.dispose();
    }
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: kLightBg,
      resizeToAvoidBottomInset: true,
      body: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          const StatusBar(),
          AppHeaderBar(
            title: 'Fixed Position',
            onBack: () => Navigator.pop(context),
          ),
          Expanded(
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                const _TiersPanel(),
                Expanded(child: _FormPanel(
                  cs: _cs,
                  onCsChange: (v) => setState(() => _cs = v),
                  idCtrl: _idCtrl,
                  eastCtrl: _eastCtrl,
                  northCtrl: _northCtrl,
                  heightOdnCtrl: _heightOdnCtrl,
                  latCtrl: _latCtrl,
                  lonCtrl: _lonCtrl,
                  heightCtrl: _heightCtrl,
                  onSave: () => Navigator.pop(context),
                  onClear: () => Navigator.pop(context),
                  onCancel: () => Navigator.pop(context),
                )),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

// ── Left panel — accuracy tiers ───────────────────────────────────────────────

class _TiersPanel extends StatelessWidget {
  const _TiersPanel();

  @override
  Widget build(BuildContext context) {
    return Container(
      width: 320,
      decoration: const BoxDecoration(
        color: Color(0xFFF5F5F5),
        border: Border(right: BorderSide(color: Color(0xFFCCCCCC))),
      ),
      padding: const EdgeInsets.all(20),
      child: SingleChildScrollView(
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
          Text(
            'Position Accuracy Tiers',
            style: GoogleFonts.montserrat(
              fontSize: 18, fontWeight: FontWeight.w700,
              color: const Color(0xFF222222)),
          ),
          const SizedBox(height: 16),
          // Survey-in — currently active
          _TierCard(
            title: 'Survey-in',
            desc: '~0.5–2 m absolute accuracy.\nRelative accuracy between points: cm.',
            dotColor: const Color(0xFFE65100),
            cardBg: const Color(0xFFFFF8E1),
            textColor: const Color(0xFFE65100),
            active: true,
          ),
          const SizedBox(height: 10),
          // Monument — not yet active
          _TierCard(
            title: 'Monument / Benchmark',
            desc: 'cm absolute accuracy.\nEnter coordinates from a known surveyed point.',
            dotColor: const Color(0xFF2E7D32),
            cardBg: const Color(0xFFF1F8E9),
            textColor: const Color(0xFF2E7D32),
            active: false,
          ),
          const SizedBox(height: 10),
          // NTRIP — future
          _TierCard(
            title: 'NTRIP Network',
            desc: 'cm absolute accuracy.\nBase position from correction network.\n(Future feature — requires WiFi)',
            dotColor: const Color(0xFFAAAAAA),
            cardBg: const Color(0xFFF0F0F0),
            textColor: const Color(0xFFAAAAAA),
            active: false,
            dim: true,
          ),
        ],
        ),
      ),
    );
  }
}

class _TierCard extends StatelessWidget {
  const _TierCard({
    required this.title,
    required this.desc,
    required this.dotColor,
    required this.cardBg,
    required this.textColor,
    required this.active,
    this.dim = false,
  });

  final String title;
  final String desc;
  final Color dotColor;
  final Color cardBg;
  final Color textColor;
  final bool active;
  final bool dim;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: cardBg,
        borderRadius: BorderRadius.circular(8),
        border: Border.all(
          color: active ? textColor : Colors.transparent,
          width: 2,
        ),
      ),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Container(
            width: 14, height: 14,
            margin: const EdgeInsets.only(top: 3),
            decoration: BoxDecoration(color: dotColor, shape: BoxShape.circle),
          ),
          const SizedBox(width: 12),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(title,
                    style: GoogleFonts.montserrat(
                        fontSize: 16, fontWeight: FontWeight.w700,
                        color: textColor)),
                const SizedBox(height: 4),
                Text(desc,
                    style: GoogleFonts.montserrat(
                        fontSize: 13, color: const Color(0xFF777777),
                        height: 1.4)),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

// ── Right panel — coordinate entry form ───────────────────────────────────────

class _FormPanel extends StatelessWidget {
  const _FormPanel({
    required this.cs,
    required this.onCsChange,
    required this.idCtrl,
    required this.eastCtrl,
    required this.northCtrl,
    required this.heightOdnCtrl,
    required this.latCtrl,
    required this.lonCtrl,
    required this.heightCtrl,
    required this.onSave,
    required this.onClear,
    required this.onCancel,
  });

  final _CoordSystem cs;
  final ValueChanged<_CoordSystem> onCsChange;
  final TextEditingController idCtrl;
  final TextEditingController eastCtrl;
  final TextEditingController northCtrl;
  final TextEditingController heightOdnCtrl;
  final TextEditingController latCtrl;
  final TextEditingController lonCtrl;
  final TextEditingController heightCtrl;
  final VoidCallback onSave;
  final VoidCallback onClear;
  final VoidCallback onCancel;

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(24),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text('Enter Monument / Benchmark',
              style: GoogleFonts.montserrat(
                  fontSize: 22, fontWeight: FontWeight.w700,
                  color: const Color(0xFF222222))),
          const SizedBox(height: 16),

          // Monument ID
          _FormLabel('Monument / Benchmark ID (optional)'),
          const SizedBox(height: 6),
          _FormField(controller: idCtrl, hint: 'e.g. OS-BM-TP-1234 or Site datum A'),
          const SizedBox(height: 14),

          // Coordinate system toggle
          _FormLabel('Coordinate System'),
          const SizedBox(height: 6),
          _CsToggle(selected: cs, onChanged: onCsChange),
          const SizedBox(height: 14),

          // Coordinate fields
          if (cs == _CoordSystem.osgb36)
            Row(
              children: [
                Expanded(child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                  _FormLabel('Easting (m)'),
                  const SizedBox(height: 6),
                  _FormField(controller: eastCtrl, numeric: true),
                ])),
                const SizedBox(width: 12),
                Expanded(child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                  _FormLabel('Northing (m)'),
                  const SizedBox(height: 6),
                  _FormField(controller: northCtrl, numeric: true),
                ])),
                const SizedBox(width: 12),
                Expanded(child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                  _FormLabel('Height ODN (m)'),
                  const SizedBox(height: 6),
                  _FormField(controller: heightOdnCtrl, numeric: true),
                ])),
              ],
            )
          else
            Row(
              children: [
                Expanded(child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                  _FormLabel('Latitude (°)'),
                  const SizedBox(height: 6),
                  _FormField(controller: latCtrl, hint: 'e.g. 52.657570', numeric: true, signed: true),
                ])),
                const SizedBox(width: 12),
                Expanded(child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                  _FormLabel('Longitude (°)'),
                  const SizedBox(height: 6),
                  _FormField(controller: lonCtrl, hint: 'e.g. 1.716573', numeric: true, signed: true),
                ])),
                const SizedBox(width: 12),
                Expanded(child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                  _FormLabel('Height (m)'),
                  const SizedBox(height: 6),
                  _FormField(controller: heightCtrl, hint: 'e.g. 63.180', numeric: true, signed: true),
                ])),
              ],
            ),

          const SizedBox(height: 20),

          Row(
            children: [
              Expanded(
                child: ElevatedButton(
                  onPressed: onSave,
                  style: ElevatedButton.styleFrom(
                    backgroundColor: kBtnGreen,
                    foregroundColor: Colors.white,
                    elevation: 0,
                    padding: const EdgeInsets.symmetric(vertical: 14),
                    shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(8)),
                  ),
                  child: Text('Save',
                      style: GoogleFonts.montserrat(
                          fontSize: 20, fontWeight: FontWeight.w700)),
                ),
              ),
              const SizedBox(width: 10),
              Expanded(
                child: ElevatedButton(
                  onPressed: onClear,
                  style: ElevatedButton.styleFrom(
                    backgroundColor: const Color(0xFFE53935),
                    foregroundColor: Colors.white,
                    elevation: 0,
                    padding: const EdgeInsets.symmetric(vertical: 14),
                    shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(8)),
                  ),
                  child: Text('Clear Position',
                      style: GoogleFonts.montserrat(
                          fontSize: 20, fontWeight: FontWeight.w700)),
                ),
              ),
              const SizedBox(width: 10),
              Expanded(
                child: ElevatedButton(
                  onPressed: onCancel,
                  style: ElevatedButton.styleFrom(
                    backgroundColor: const Color(0xFF757575),
                    foregroundColor: Colors.white,
                    elevation: 0,
                    padding: const EdgeInsets.symmetric(vertical: 14),
                    shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(8)),
                  ),
                  child: Text('Cancel',
                      style: GoogleFonts.montserrat(
                          fontSize: 20, fontWeight: FontWeight.w700)),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }
}

class _FormLabel extends StatelessWidget {
  const _FormLabel(this.text);
  final String text;

  @override
  Widget build(BuildContext context) {
    return Text(text,
        style: GoogleFonts.montserrat(
            fontSize: 14, fontWeight: FontWeight.w700,
            color: const Color(0xFF555555)));
  }
}

class _FormField extends StatelessWidget {
  const _FormField({
    required this.controller,
    this.hint,
    this.numeric = false,
    this.signed = false,
  });

  final TextEditingController controller;
  final String? hint;
  final bool numeric;
  final bool signed;

  @override
  Widget build(BuildContext context) {
    return TextField(
      controller: controller,
      keyboardType: numeric
          ? TextInputType.numberWithOptions(decimal: true, signed: signed)
          : TextInputType.text,
      style: GoogleFonts.montserrat(fontSize: 18, color: kLightText),
      decoration: InputDecoration(
        hintText: hint,
        hintStyle: GoogleFonts.montserrat(color: kLightTextMuted),
        contentPadding:
            const EdgeInsets.symmetric(horizontal: 12, vertical: 10),
        border: OutlineInputBorder(
          borderRadius: BorderRadius.circular(8),
          borderSide: const BorderSide(color: Color(0xFFCCCCCC), width: 2),
        ),
        enabledBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(8),
          borderSide: const BorderSide(color: Color(0xFFCCCCCC), width: 2),
        ),
        focusedBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(8),
          borderSide: const BorderSide(color: kBlueAccent, width: 2),
        ),
      ),
    );
  }
}

class _CsToggle extends StatelessWidget {
  const _CsToggle({required this.selected, required this.onChanged});
  final _CoordSystem selected;
  final ValueChanged<_CoordSystem> onChanged;

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 40,
      decoration: BoxDecoration(
        border: Border.all(color: const Color(0xFFCCCCCC), width: 2),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Row(
        children: [
          Expanded(child: _CsBtn(
            label: 'OSGB36 (National Grid)',
            active: selected == _CoordSystem.osgb36,
            isLeft: true,
            onTap: () => onChanged(_CoordSystem.osgb36),
          )),
          Expanded(child: _CsBtn(
            label: 'WGS84 (GPS)',
            active: selected == _CoordSystem.wgs84,
            isLeft: false,
            onTap: () => onChanged(_CoordSystem.wgs84),
          )),
        ],
      ),
    );
  }
}

class _CsBtn extends StatelessWidget {
  const _CsBtn({
    required this.label,
    required this.active,
    required this.isLeft,
    required this.onTap,
  });

  final String label;
  final bool active;
  final bool isLeft;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        alignment: Alignment.center,
        decoration: BoxDecoration(
          color: active ? kBlueAccent : const Color(0xFFF5F5F5),
          borderRadius: BorderRadius.horizontal(
            left: isLeft ? const Radius.circular(6) : Radius.zero,
            right: isLeft ? Radius.zero : const Radius.circular(6),
          ),
        ),
        child: Text(
          label,
          style: GoogleFonts.montserrat(
            fontSize: 14,
            fontWeight: FontWeight.w700,
            color: active ? Colors.white : const Color(0xFF888888),
          ),
        ),
      ),
    );
  }
}
