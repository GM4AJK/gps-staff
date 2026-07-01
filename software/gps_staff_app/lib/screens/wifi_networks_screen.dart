import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';
import '../widgets/app_header_bar.dart';
import 'password_entry_screen.dart';

enum _NetState { connected, inRange, outOfRange }

class WifiNetworksScreen extends StatelessWidget {
  const WifiNetworksScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: kLightBg,
      body: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          const StatusBar(),
          AppHeaderBar(
              title: 'WiFi Settings',
              onBack: () => Navigator.pop(context)),
          Expanded(
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: const [
                _LeftPanel(),
                _RightPanel(),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

// ── Left panel ────────────────────────────────────────────────────────────────

class _LeftPanel extends StatelessWidget {
  const _LeftPanel();

  @override
  Widget build(BuildContext context) {
    return Container(
      width: 340,
      decoration: const BoxDecoration(
        color: Color(0xFFF5F5F5),
        border: Border(right: BorderSide(color: Color(0xFFCCCCCC))),
      ),
      padding: const EdgeInsets.all(20),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          _PanelTitle('Current Config'),
          const SizedBox(height: 10),
          _ConfigTable(rows: const [
            _Row('Network SSID:', 'BT-HomeHub-ABC'),
            _Row('Channel:', '6'),
            _Row('Status:', 'Connected', valueColor: Color(0xFF43A047)),
            _Row('IP Address:', '192.168.1.42'),
            _Row('Signal:', '−65 dBm'),
          ]),
          const SizedBox(height: 20),
          _PanelTitle('Remembered'),
          const SizedBox(height: 10),
          _ConfigTable(rows: const [
            _Row('Slots used:', '3 of 5'),
          ]),
          const SizedBox(height: 8),
          Text(
            'On power-on, Base connects automatically to the '
            'highest-signal remembered network in range.',
            style: GoogleFonts.montserrat(
                fontSize: 11,
                color: kLightTextMuted,
                height: 1.6),
          ),
        ],
      ),
    );
  }
}

class _PanelTitle extends StatelessWidget {
  const _PanelTitle(this.text);
  final String text;

  @override
  Widget build(BuildContext context) {
    return Text(text,
        style: GoogleFonts.montserrat(
            fontSize: 20,
            fontWeight: FontWeight.w700,
            color: const Color(0xFF222222)));
  }
}

class _Row {
  const _Row(this.label, this.value, {this.valueColor});
  final String label;
  final String value;
  final Color? valueColor;
}

class _ConfigTable extends StatelessWidget {
  const _ConfigTable({required this.rows});
  final List<_Row> rows;

  @override
  Widget build(BuildContext context) {
    return Column(
      children: rows
          .map((r) => Padding(
                padding: const EdgeInsets.symmetric(vertical: 5),
                child: Row(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    SizedBox(
                      width: 160,
                      child: Text(r.label,
                          style: GoogleFonts.montserrat(
                              fontSize: 14,
                              fontWeight: FontWeight.w700,
                              color: const Color(0xFF555555))),
                    ),
                    Expanded(
                      child: Text(r.value,
                          style: GoogleFonts.montserrat(
                              fontSize: 14,
                              fontWeight: r.valueColor != null
                                  ? FontWeight.w700
                                  : FontWeight.w400,
                              color: r.valueColor ?? kLightText)),
                    ),
                  ],
                ),
              ))
          .toList(),
    );
  }
}

// ── Right panel ───────────────────────────────────────────────────────────────

class _RightPanel extends StatelessWidget {
  const _RightPanel();

  @override
  Widget build(BuildContext context) {
    return Expanded(
      child: SingleChildScrollView(
        padding: const EdgeInsets.fromLTRB(20, 16, 20, 16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            _SectionHeader(label: 'Remembered Networks'),
            _RememberedRow(
                ssid: 'BT-HomeHub-ABC',
                meta: 'Ch 6 · −65 dBm',
                state: _NetState.connected),
            _RememberedRow(
                ssid: 'AndroidAP_Kirk',
                meta: 'Ch 11 · −72 dBm',
                state: _NetState.inRange),
            _RememberedRow(
                ssid: 'VM12345678',
                meta: 'Ch — · —',
                state: _NetState.outOfRange),
            const SizedBox(height: 10),
            _SectionHeader(
              label: 'Other Networks',
              trailing: _ScanButton(onPressed: () {}),
            ),
            _OtherNetRow(ssid: 'SKYxxxx1', meta: 'Ch 12 · −79 dBm', channel: 12),
            _OtherNetRow(ssid: 'SKYxxxx2', meta: 'Ch 2 · −89 dBm', channel: 2),
          ],
        ),
      ),
    );
  }
}

// ── Section header ────────────────────────────────────────────────────────────

class _SectionHeader extends StatelessWidget {
  const _SectionHeader({required this.label, this.trailing});
  final String label;
  final Widget? trailing;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 5),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(
            label.toUpperCase(),
            style: GoogleFonts.montserrat(
                fontSize: 10,
                fontWeight: FontWeight.w700,
                letterSpacing: 1.0,
                color: const Color(0xFFAAAAAA)),
          ),
          if (trailing != null) trailing!,
        ],
      ),
    );
  }
}

class _ScanButton extends StatelessWidget {
  const _ScanButton({required this.onPressed});
  final VoidCallback onPressed;

  @override
  Widget build(BuildContext context) {
    return OutlinedButton(
      onPressed: onPressed,
      style: OutlinedButton.styleFrom(
        foregroundColor: kBlueAccent,
        side: const BorderSide(color: Color(0xFFBBDEFB)),
        padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
        minimumSize: Size.zero,
        tapTargetSize: MaterialTapTargetSize.shrinkWrap,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(5)),
      ),
      child: Text('Scan ↺',
          style: GoogleFonts.montserrat(
              fontSize: 11, fontWeight: FontWeight.w700)),
    );
  }
}

// ── Network rows ──────────────────────────────────────────────────────────────

class _RememberedRow extends StatelessWidget {
  const _RememberedRow({
    required this.ssid,
    required this.meta,
    required this.state,
  });

  final String ssid;
  final String meta;
  final _NetState state;

  @override
  Widget build(BuildContext context) {
    final isConnected = state == _NetState.connected;
    final isOut = state == _NetState.outOfRange;

    final dotColor = isConnected ? const Color(0xFF43A047) : const Color(0xFFE0E0E0);
    final ssidColor = isConnected
        ? const Color(0xFFAAAAAA)
        : isOut
            ? const Color(0xFFCCCCCC)
            : kLightText;
    final metaColor = isOut ? const Color(0xFFDDDDDD) : kLightTextMuted;

    return Container(
      decoration: const BoxDecoration(
        border: Border(bottom: BorderSide(color: Color(0xFFF0F0F0))),
      ),
      constraints: const BoxConstraints(minHeight: 44),
      padding: const EdgeInsets.symmetric(vertical: 9),
      child: Row(
        children: [
          Container(
            width: 10,
            height: 10,
            decoration: BoxDecoration(color: dotColor, shape: BoxShape.circle),
          ),
          const SizedBox(width: 10),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(ssid,
                    style: GoogleFonts.montserrat(
                        fontSize: 14,
                        fontWeight: FontWeight.w700,
                        color: ssidColor,
                        fontStyle: isConnected
                            ? FontStyle.italic
                            : FontStyle.normal)),
                Text(meta,
                    style: GoogleFonts.montserrat(
                        fontSize: 12, color: metaColor)),
              ],
            ),
          ),
          const SizedBox(width: 10),
          if (isConnected)
            _NetButton(
                label: 'Disconnect',
                bg: const Color(0xFFE53935),
                onPressed: () {}),
          if (state == _NetState.inRange) ...[
            _NetButton(
                label: 'Connect',
                bg: kBlueAccent,
                onPressed: () {}),
            const SizedBox(width: 6),
            _ForgetButton(onPressed: () {}),
          ],
          if (isOut) ...[
            Text('Not in range',
                style: GoogleFonts.montserrat(
                    fontSize: 11,
                    color: const Color(0xFFCCCCCC),
                    fontStyle: FontStyle.italic)),
            const SizedBox(width: 6),
            _ForgetButton(onPressed: () {}),
          ],
        ],
      ),
    );
  }
}

class _OtherNetRow extends StatelessWidget {
  const _OtherNetRow({required this.ssid, required this.meta, required this.channel});
  final String ssid;
  final String meta;
  final int channel;

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: const BoxDecoration(
        border: Border(bottom: BorderSide(color: Color(0xFFF0F0F0))),
      ),
      constraints: const BoxConstraints(minHeight: 44),
      padding: const EdgeInsets.symmetric(vertical: 9),
      child: Row(
        children: [
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(ssid,
                    style: GoogleFonts.montserrat(
                        fontSize: 14,
                        fontWeight: FontWeight.w700,
                        color: kLightText)),
                Text(meta,
                    style: GoogleFonts.montserrat(
                        fontSize: 12, color: kLightTextMuted)),
              ],
            ),
          ),
          const SizedBox(width: 10),
          _NetButton(
              label: 'Connect',
              bg: kBlueAccent,
              onPressed: () => Navigator.push(
                    context,
                    MaterialPageRoute(
                      builder: (_) => PasswordEntryScreen(
                          ssid: ssid, channel: channel),
                    ),
                  )),
        ],
      ),
    );
  }
}

class _NetButton extends StatelessWidget {
  const _NetButton(
      {required this.label, required this.bg, required this.onPressed});
  final String label;
  final Color bg;
  final VoidCallback onPressed;

  @override
  Widget build(BuildContext context) {
    return ElevatedButton(
      onPressed: onPressed,
      style: ElevatedButton.styleFrom(
        backgroundColor: bg,
        foregroundColor: Colors.white,
        elevation: 0,
        padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 7),
        minimumSize: Size.zero,
        tapTargetSize: MaterialTapTargetSize.shrinkWrap,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(6)),
      ),
      child: Text(label,
          style: GoogleFonts.montserrat(
              fontSize: 12, fontWeight: FontWeight.w700)),
    );
  }
}

class _ForgetButton extends StatelessWidget {
  const _ForgetButton({required this.onPressed});
  final VoidCallback onPressed;

  @override
  Widget build(BuildContext context) {
    return OutlinedButton(
      onPressed: onPressed,
      style: OutlinedButton.styleFrom(
        foregroundColor: const Color(0xFFE53935),
        side: const BorderSide(color: Color(0xFFFFCDD2)),
        padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 5),
        minimumSize: Size.zero,
        tapTargetSize: MaterialTapTargetSize.shrinkWrap,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(5)),
      ),
      child: Text('Forget',
          style: GoogleFonts.montserrat(
              fontSize: 11, fontWeight: FontWeight.w700)),
    );
  }
}
