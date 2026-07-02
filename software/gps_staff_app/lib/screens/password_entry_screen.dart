import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';
import '../widgets/app_header_bar.dart';

class PasswordEntryScreen extends StatefulWidget {
  const PasswordEntryScreen({
    super.key,
    required this.ssid,
    required this.channel,
  });

  final String ssid;
  final int channel;

  @override
  State<PasswordEntryScreen> createState() => _PasswordEntryScreenState();
}

class _PasswordEntryScreenState extends State<PasswordEntryScreen> {
  final _passwordController = TextEditingController();
  bool _obscure = true;
  bool _connecting = false;

  @override
  void dispose() {
    _passwordController.dispose();
    super.dispose();
  }

  void _connect() {
    if (_passwordController.text.isEmpty) return;
    FocusScope.of(context).unfocus();
    setState(() => _connecting = true);
    // TODO: send SSID + password to Base via BLE
    // On success: Navigator.pop(context) back to WiFi Networks
    // On failure: setState(() => _connecting = false) + show error
  }

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
            title: 'Enter Password',
            onBack: () => Navigator.pop(context),
          ),
          Expanded(
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                _LeftPanel(connecting: _connecting, ssid: widget.ssid),
                Expanded(
                  child: Opacity(
                    opacity: _connecting ? 0.5 : 1.0,
                    child: _RightPanel(
                      ssid: widget.ssid,
                      channel: widget.channel,
                      controller: _passwordController,
                      obscure: _obscure,
                      connecting: _connecting,
                      onToggleObscure: () =>
                          setState(() => _obscure = !_obscure),
                      onConnect: _connect,
                      onCancel: () => Navigator.pop(context),
                    ),
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

// ── Left panel ────────────────────────────────────────────────────────────────

class _LeftPanel extends StatelessWidget {
  const _LeftPanel({required this.connecting, required this.ssid});
  final bool connecting;
  final String ssid;

  @override
  Widget build(BuildContext context) {
    return Container(
      width: 340,
      decoration: const BoxDecoration(
        color: Color(0xFFF5F5F5),
        border: Border(right: BorderSide(color: Color(0xFFCCCCCC))),
      ),
      padding: const EdgeInsets.all(20),
      child: connecting ? _connectingState() : _networkInfoState(),
    );
  }

  Widget _connectingState() {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        const SizedBox(
          width: 64,
          height: 64,
          child: CircularProgressIndicator(
            strokeWidth: 6,
            color: kBlueAccent,
          ),
        ),
        const SizedBox(height: 20),
        Text(
          'Connecting to $ssid',
          style: const TextStyle(
            fontSize: 20,
            fontWeight: FontWeight.w700,
            color: Color(0xFF555555),
          ),
          textAlign: TextAlign.center,
        ),
        const SizedBox(height: 8),
        const Text(
          'Waiting for Base to confirm…',
          style: TextStyle(fontSize: 16, color: Color(0xFF888888)),
          textAlign: TextAlign.center,
        ),
      ],
    );
  }

  Widget _networkInfoState() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          'Connect to Network',
          style: GoogleFonts.montserrat(
            fontSize: 20,
            fontWeight: FontWeight.w700,
            color: const Color(0xFF222222),
          ),
        ),
        const SizedBox(height: 16),
        Text(
          'Enter the WiFi password for this network. '
          'The password will be stored securely on the Base unit.',
          style: GoogleFonts.montserrat(
              fontSize: 14, color: kLightTextMuted, height: 1.5),
        ),
        const SizedBox(height: 20),
        _InfoRow('Network:', ssid),
        const _InfoRow('Security:', 'WPA2-Personal'),
      ],
    );
  }
}

class _InfoRow extends StatelessWidget {
  const _InfoRow(this.label, this.value);
  final String label;
  final String value;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 5),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          SizedBox(
            width: 100,
            child: Text(label,
                style: GoogleFonts.montserrat(
                    fontSize: 14,
                    fontWeight: FontWeight.w700,
                    color: const Color(0xFF555555))),
          ),
          Expanded(
            child: Text(value,
                style: GoogleFonts.montserrat(
                    fontSize: 14, color: kLightText)),
          ),
        ],
      ),
    );
  }
}

// ── Right panel ───────────────────────────────────────────────────────────────

class _RightPanel extends StatelessWidget {
  const _RightPanel({
    required this.ssid,
    required this.channel,
    required this.controller,
    required this.obscure,
    required this.connecting,
    required this.onToggleObscure,
    required this.onConnect,
    required this.onCancel,
  });

  final String ssid;
  final int channel;
  final TextEditingController controller;
  final bool obscure;
  final bool connecting;
  final VoidCallback onToggleObscure;
  final VoidCallback onConnect;
  final VoidCallback onCancel;

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(20),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisSize: MainAxisSize.min,
        children: [
          Text(
            'Enter Password',
            style: GoogleFonts.montserrat(
              fontSize: 24,
              fontWeight: FontWeight.w700,
              color: const Color(0xFF222222),
            ),
          ),
          const SizedBox(height: 8),
          Text(
            'Network: $ssid  ·  Channel: $channel',
            style: GoogleFonts.montserrat(
                fontSize: 16, color: kLightTextSub),
          ),
          const SizedBox(height: 20),
          Text(
            'WiFi Password',
            style: GoogleFonts.montserrat(
              fontSize: 16,
              fontWeight: FontWeight.w700,
              color: const Color(0xFF555555),
            ),
          ),
          const SizedBox(height: 8),
          Row(
            children: [
              Expanded(
                child: TextField(
                  controller: controller,
                  obscureText: obscure,
                  enabled: !connecting,
                  autofocus: true,
                  style: GoogleFonts.montserrat(fontSize: 20, color: kLightText),
                  decoration: InputDecoration(
                    hintText: 'Enter password…',
                    hintStyle: GoogleFonts.montserrat(
                        color: const Color(0xFFCCCCCC)),
                    contentPadding: const EdgeInsets.symmetric(
                        horizontal: 12, vertical: 12),
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
                ),
              ),
              const SizedBox(width: 10),
              SizedBox(
                height: 48,
                child: OutlinedButton(
                  onPressed: connecting ? null : onToggleObscure,
                  style: OutlinedButton.styleFrom(
                    side: const BorderSide(color: Color(0xFFCCCCCC), width: 2),
                    shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(8)),
                    padding: const EdgeInsets.symmetric(horizontal: 20),
                    backgroundColor: const Color(0xFFEEEEEE),
                  ),
                  child: Text(
                    obscure ? 'Show' : 'Hide',
                    style: GoogleFonts.montserrat(
                      fontSize: 16,
                      fontWeight: FontWeight.w700,
                      color: kLightText,
                    ),
                  ),
                ),
              ),
            ],
          ),
          const SizedBox(height: 16),
          Row(
            children: [
              ElevatedButton(
                onPressed: connecting ? null : onConnect,
                style: ElevatedButton.styleFrom(
                  backgroundColor: kBlueAccent,
                  foregroundColor: Colors.white,
                  elevation: 0,
                  padding: const EdgeInsets.symmetric(
                      horizontal: 32, vertical: 14),
                  shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(8)),
                ),
                child: Text('Connect',
                    style: GoogleFonts.montserrat(
                        fontSize: 20, fontWeight: FontWeight.w700)),
              ),
              const SizedBox(width: 12),
              ElevatedButton(
                onPressed: connecting ? null : onCancel,
                style: ElevatedButton.styleFrom(
                  backgroundColor: const Color(0xFF757575),
                  foregroundColor: Colors.white,
                  elevation: 0,
                  padding: const EdgeInsets.symmetric(
                      horizontal: 32, vertical: 14),
                  shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(8)),
                ),
                child: Text('Cancel',
                    style: GoogleFonts.montserrat(
                        fontSize: 20, fontWeight: FontWeight.w700)),
              ),
            ],
          ),
        ],
      ),
    );
  }
}
