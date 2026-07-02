import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';
import 'mode5_remote_listener_screen.dart';
import 'mode5_mobile_connector_screen.dart';

const _modeNames = {
	1: 'Local RF',
	2: 'Rover NTRIP',
	3: 'Base via Relay',
	4: 'Dual NTRIP',
	5: 'Direct TCP',
	6: 'Fixed Base',
};

class OperatingModeScreen extends StatefulWidget {
	const OperatingModeScreen({super.key});

	@override
	State<OperatingModeScreen> createState() => _OperatingModeScreenState();
}

class _OperatingModeScreenState extends State<OperatingModeScreen> {
	int _selected = 1;

	@override
	Widget build(BuildContext context) {
		return Scaffold(
			backgroundColor: kBgHome,
      resizeToAvoidBottomInset: false,
			body: Column(
				crossAxisAlignment: CrossAxisAlignment.stretch,
				children: [
					const StatusBar(),
					_Header(
						selected: _selected,
						onBack: () => Navigator.pop(context),
					),
					Expanded(
						child: SingleChildScrollView(
							padding: const EdgeInsets.fromLTRB(24, 10, 24, 10),
							child: Column(
								children: [
									_ModeCard(
										mode: 1,
										name: 'Local RF Link',
										desc: 'Base at benchmark surveys-in and broadcasts RTCM to Rover over radio. RF mode configured in Base Config.',
										chips: const [_Chip('RF only', _ChipStyle.rf), _Chip('No internet', _ChipStyle.rf)],
										selected: _selected == 1,
										onTap: () => setState(() => _selected = 1),
									),
									const SizedBox(height: 6),
									_ModeCard(
										mode: 2,
										name: 'Rover NTRIP Direct',
										desc: 'Rover connects to an internet NTRIP caster (e.g. OS Net) for corrections. Base not required.',
										chips: const [_Chip('WiFi — Rover', _ChipStyle.wifi), _Chip('NTRIP account', _ChipStyle.ntrip)],
										selected: _selected == 2,
										onTap: () => setState(() => _selected = 2),
									),
									const SizedBox(height: 6),
									_ModeCard(
										mode: 3,
										name: 'Base via Relay Caster',
										desc: 'Base streams RTCM to a relay caster. Rover pulls corrections from the same caster over internet.',
										chips: const [_Chip('Hotspot — Base', _ChipStyle.hotspot), _Chip('WiFi — Rover', _ChipStyle.wifi), _Chip('Relay caster', _ChipStyle.relay)],
										selected: _selected == 3,
										onTap: () => setState(() => _selected = 3),
									),
									const SizedBox(height: 6),
									_ModeCard(
										mode: 4,
										name: 'Dual NTRIP Independent',
										desc: 'Base and Rover each connect independently to the same CORS NTRIP service. No direct device link.',
										chips: const [_Chip('WiFi — both', _ChipStyle.wifi), _Chip('NTRIP account', _ChipStyle.ntrip)],
										selected: _selected == 4,
										onTap: () => setState(() => _selected = 4),
									),
									const SizedBox(height: 6),
									_ModeCard(
										mode: 5,
										name: 'Direct TCP',
										desc: 'One device listens at home via a router port forward; the other connects from the field. No relay required.',
										chips: const [_Chip('WiFi — remote', _ChipStyle.wifi), _Chip('Hotspot — mobile', _ChipStyle.hotspot), _Chip('Port forward', _ChipStyle.portfwd)],
										selected: _selected == 5,
										onTap: () => setState(() => _selected = 5),
									),
									const SizedBox(height: 6),
									_ModeCard(
										mode: 6,
										name: 'Fixed Base Station',
										desc: 'Permanent Base at a known surveyed position. Streams RTCM to NTRIP caster 24/7. Optional concurrent RF broadcast.',
										chips: const [_Chip('Fixed install', _ChipStyle.fixed), _Chip('WiFi — mains', _ChipStyle.wifi), _Chip('NTRIP stream', _ChipStyle.ntrip), _Chip('RF optional', _ChipStyle.rf)],
										selected: _selected == 6,
										onTap: () => setState(() => _selected = 6),
									),
								],
							),
						),
					),
					_ConfirmBar(selected: _selected),
				],
			),
		);
	}
}

// ── Header ────────────────────────────────────────────────────────────────────

class _Header extends StatelessWidget {
	const _Header({required this.selected, required this.onBack});
	final int selected;
	final VoidCallback onBack;

	@override
	Widget build(BuildContext context) {
		return Container(
			height: 52,
			color: kBgStatus,
			padding: const EdgeInsets.symmetric(horizontal: 16),
			child: Row(
				children: [
					GestureDetector(
						onTap: onBack,
						behavior: HitTestBehavior.opaque,
						child: Padding(
							padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
							child: Text('←',
									style: GoogleFonts.montserrat(
											fontSize: 24, color: kTextMuted, height: 1)),
						),
					),
					const SizedBox(width: 14),
					Expanded(
						child: Text('Operating Mode',
								style: GoogleFonts.montserrat(
										fontSize: 18,
										fontWeight: FontWeight.w700,
										color: kTextPrimary)),
					),
					Container(
						padding: const EdgeInsets.symmetric(
								horizontal: 10, vertical: 3),
						decoration: BoxDecoration(
							color: const Color(0xFF1A2640),
							border: Border.all(color: kBlueButton),
							borderRadius: BorderRadius.circular(4),
						),
						child: Text(
							'Mode $selected — ${_modeNames[selected]}',
							style: GoogleFonts.montserrat(
								fontSize: 11,
								fontWeight: FontWeight.w700,
								color: const Color(0xFF64B5F6),
								fontFeatures: const [FontFeature.tabularFigures()],
							),
						),
					),
				],
			),
		);
	}
}

// ── Mode card ─────────────────────────────────────────────────────────────────

class _ModeCard extends StatelessWidget {
	const _ModeCard({
		required this.mode,
		required this.name,
		required this.desc,
		required this.chips,
		required this.selected,
		required this.onTap,
	});

	final int mode;
	final String name;
	final String desc;
	final List<_Chip> chips;
	final bool selected;
	final VoidCallback onTap;

	@override
	Widget build(BuildContext context) {
		return GestureDetector(
			onTap: onTap,
			child: AnimatedContainer(
				duration: const Duration(milliseconds: 120),
				decoration: BoxDecoration(
					color: selected
							? const Color(0xFF2196F3).withValues(alpha: 0.08)
							: Colors.white.withValues(alpha: 0.02),
					borderRadius: BorderRadius.circular(8),
					border: Border.all(
						color: selected
								? const Color(0xFF2196F3).withValues(alpha: 0.2)
								: Colors.white.withValues(alpha: 0.06),
					),
				),
				child: Stack(
					children: [
						// Left accent bar — separate widget to avoid non-uniform border+radius clash
						if (selected)
							Positioned(
								left: 0, top: 0, bottom: 0,
								child: Container(
									width: 3,
									decoration: const BoxDecoration(
										color: kBlueAccent,
										borderRadius: BorderRadius.only(
											topLeft: Radius.circular(8),
											bottomLeft: Radius.circular(8),
										),
									),
								),
							),
						Padding(
							padding: const EdgeInsets.fromLTRB(14, 9, 16, 9),
							child: Column(
								crossAxisAlignment: CrossAxisAlignment.start,
								children: [
									Row(
										children: [
											_RadioDot(selected: selected),
											const SizedBox(width: 12),
											_ModeNum(mode: mode, selected: selected),
											const SizedBox(width: 12),
											Expanded(
												child: Text(
													name,
													style: GoogleFonts.montserrat(
														fontSize: 14,
														fontWeight: FontWeight.w700,
														color: selected
																? const Color(0xFF90CAF9)
																: kTextSub,
													),
												),
											),
											Wrap(
												spacing: 5,
												children: chips,
											),
										],
									),
									const SizedBox(height: 4),
									Padding(
										padding: const EdgeInsets.only(left: 40),
										child: Text(
											desc,
											style: GoogleFonts.montserrat(
												fontSize: 11,
												color: selected ? kTextMuted : const Color(0xFF37474F),
												height: 1.5,
											),
										),
									),
								],
							),
						),
					],
				),
			),
		);
	}
}

class _RadioDot extends StatelessWidget {
	const _RadioDot({required this.selected});
	final bool selected;

	@override
	Widget build(BuildContext context) {
		return Container(
			width: 16,
			height: 16,
			decoration: BoxDecoration(
				shape: BoxShape.circle,
				border: Border.all(
					color: selected ? kBlueAccent : const Color(0xFF37474F),
					width: 2,
				),
			),
			child: selected
					? Center(
							child: Container(
								width: 7,
								height: 7,
								decoration: const BoxDecoration(
									color: kBlueAccent,
									shape: BoxShape.circle,
								),
							),
						)
					: null,
		);
	}
}

class _ModeNum extends StatelessWidget {
	const _ModeNum({required this.mode, required this.selected});
	final int mode;
	final bool selected;

	@override
	Widget build(BuildContext context) {
		return Container(
			padding: const EdgeInsets.symmetric(horizontal: 7, vertical: 2),
			decoration: BoxDecoration(
				color: const Color(0xFF161F2A),
				border: Border.all(
						color: selected
								? const Color(0xFF263238)
								: const Color(0xFF1E2D3D)),
				borderRadius: BorderRadius.circular(4),
			),
			child: Text(
				'$mode',
				style: GoogleFonts.montserrat(
					fontSize: 10,
					fontWeight: FontWeight.w700,
					color: selected
							? const Color(0xFF546E7A)
							: const Color(0xFF37474F),
					fontFeatures: const [FontFeature.tabularFigures()],
				),
			),
		);
	}
}

// ── Requirement chips ─────────────────────────────────────────────────────────

enum _ChipStyle { rf, wifi, ntrip, relay, portfwd, hotspot, fixed }

class _Chip extends StatelessWidget {
	const _Chip(this.label, this.style);
	final String label;
	final _ChipStyle style;

	({Color bg, Color fg, Color border}) get _colors => switch (style) {
		_ChipStyle.rf      => (bg: const Color(0xFF1B2631), fg: const Color(0xFF546E7A), border: const Color(0xFF263238)),
		_ChipStyle.wifi    => (bg: const Color(0xFF0D2137), fg: const Color(0xFF64B5F6), border: const Color(0xFF1565C0)),
		_ChipStyle.ntrip   => (bg: const Color(0xFF2D1A00), fg: const Color(0xFFFF8F00), border: const Color(0xFFE65100)),
		_ChipStyle.relay   => (bg: const Color(0xFF2D1A00), fg: const Color(0xFFFFA726), border: const Color(0xFFBF360C)),
		_ChipStyle.portfwd => (bg: const Color(0xFF1A0A2E), fg: const Color(0xFFCE93D8), border: const Color(0xFF4A148C)),
		_ChipStyle.hotspot => (bg: const Color(0xFF00251A), fg: const Color(0xFF4DB6AC), border: const Color(0xFF00695C)),
		_ChipStyle.fixed   => (bg: const Color(0xFF1A2010), fg: const Color(0xFFAED581), border: const Color(0xFF558B2F)),
	};

	@override
	Widget build(BuildContext context) {
		final c = _colors;
		return Container(
			padding: const EdgeInsets.symmetric(horizontal: 7, vertical: 2),
			decoration: BoxDecoration(
				color: c.bg,
				border: Border.all(color: c.border),
				borderRadius: BorderRadius.circular(10),
			),
			child: Text(
				label.toUpperCase(),
				style: GoogleFonts.montserrat(
					fontSize: 9,
					fontWeight: FontWeight.w700,
					color: c.fg,
					letterSpacing: 0.4,
				),
			),
		);
	}
}

// ── Confirm bar ───────────────────────────────────────────────────────────────

class _ConfirmBar extends StatelessWidget {
	const _ConfirmBar({required this.selected});
	final int selected;

	@override
	Widget build(BuildContext context) {
		return Container(
			padding: const EdgeInsets.fromLTRB(24, 12, 24, 12),
			decoration: BoxDecoration(
				color: kBgHome,
				border: Border(
						top: BorderSide(color: Colors.white.withValues(alpha: 0.06))),
			),
			child: SizedBox(
				height: 52,
				child: ElevatedButton(
					onPressed: () => _confirm(context),
					style: ElevatedButton.styleFrom(
						backgroundColor: kBlueButton,
						foregroundColor: Colors.white,
						elevation: 0,
						shape: RoundedRectangleBorder(
								borderRadius: BorderRadius.circular(8)),
					),
					child: Text(
						'Confirm — Configure Mode $selected →',
						style: GoogleFonts.montserrat(
								fontSize: 16, fontWeight: FontWeight.w700),
					),
				),
			),
		);
	}

	Future<void> _confirm(BuildContext context) async {
		if (selected != 5) {
			Navigator.pop(context);
			return;
		}
		// Mode 5 — ask which role this device plays
		final role = await showDialog<int>(
			context: context,
			builder: (ctx) => AlertDialog(
				backgroundColor: const Color(0xFF1A0A2E),
				title: Text(
					'Mode 5 — Direct TCP',
					style: GoogleFonts.montserrat(
						fontSize: 16,
						fontWeight: FontWeight.w700,
						color: const Color(0xFFCE93D8),
					),
				),
				content: Text(
					'Which role is this device playing?',
					style: GoogleFonts.montserrat(
						fontSize: 13,
						color: const Color(0xFF9E9E9E),
					),
				),
				actions: [
					TextButton(
						onPressed: () => Navigator.pop(ctx),
						child: Text('Cancel',
								style: GoogleFonts.montserrat(
										color: const Color(0xFF546E7A))),
					),
					ElevatedButton(
						onPressed: () => Navigator.pop(ctx, 0),
						style: ElevatedButton.styleFrom(
							backgroundColor: const Color(0xFF4A148C),
							foregroundColor: Colors.white,
						),
						child: Text('Base at home',
								style: GoogleFonts.montserrat(
										fontSize: 13, fontWeight: FontWeight.w700)),
					),
					ElevatedButton(
						onPressed: () => Navigator.pop(ctx, 1),
						style: ElevatedButton.styleFrom(
							backgroundColor: const Color(0xFF4A148C),
							foregroundColor: Colors.white,
						),
						child: Text('Rover in field',
								style: GoogleFonts.montserrat(
										fontSize: 13, fontWeight: FontWeight.w700)),
					),
				],
			),
		);
		if (role == null || !context.mounted) return;
		if (role == 0) {
			Navigator.push(context,
					MaterialPageRoute(builder: (_) => const Mode5RemoteListenerScreen()));
		} else {
			Navigator.push(context,
					MaterialPageRoute(builder: (_) => const Mode5MobileConnectorScreen()));
		}
	}
}
