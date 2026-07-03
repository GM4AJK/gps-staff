import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';
import 'start_survey_session_screen.dart';

class RoverConfigScreen extends StatefulWidget {
	const RoverConfigScreen({super.key});

	@override
	State<RoverConfigScreen> createState() => _RoverConfigScreenState();
}

class _RoverConfigScreenState extends State<RoverConfigScreen> {
	bool _receiving = true;
	String _poleHeight = '1.80';

	@override
	Widget build(BuildContext context) {
		return Scaffold(
			backgroundColor: kLightBg,
			resizeToAvoidBottomInset: false,
			body: Column(
				crossAxisAlignment: CrossAxisAlignment.stretch,
				children: [
					const StatusBar(),
					_RoverHeader(
						onBack: () => Navigator.pop(context),
						rfMode: 'GFSK',
					),
					Expanded(
						child: Row(
							crossAxisAlignment: CrossAxisAlignment.stretch,
							children: [
								_LeftPanel(receiving: _receiving),
								Expanded(
									child: _RightPanel(
										poleHeight: _poleHeight,
										receiving: _receiving,
										onToggle: () =>
												setState(() => _receiving = !_receiving),
										onPoleHeightEdit: () => _editPoleHeight(context),
										onSave: () => Navigator.push(
											context,
											MaterialPageRoute(
													builder: (_) =>
															const StartSurveySessionScreen()),
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

	Future<void> _editPoleHeight(BuildContext context) async {
		final controller = TextEditingController(text: _poleHeight);
		final result = await showDialog<String>(
			context: context,
			builder: (ctx) => AlertDialog(
				backgroundColor: kLightBg,
				title: Text('Pole Height (m)',
						style: GoogleFonts.montserrat(
								fontWeight: FontWeight.w700, color: kLightText)),
				content: TextField(
					controller: controller,
					autofocus: true,
					keyboardType:
							const TextInputType.numberWithOptions(decimal: true),
					decoration: const InputDecoration(suffixText: 'm'),
					style: GoogleFonts.montserrat(fontSize: 20, color: kLightText),
				),
				actions: [
					TextButton(
						onPressed: () => Navigator.pop(ctx),
						child: Text('Cancel',
								style: GoogleFonts.montserrat(color: kLightTextMuted)),
					),
					ElevatedButton(
						onPressed: () =>
								Navigator.pop(ctx, controller.text),
						style: ElevatedButton.styleFrom(
								backgroundColor: kBlueButton,
								foregroundColor: Colors.white),
						child: Text('Confirm',
								style: GoogleFonts.montserrat(
										fontWeight: FontWeight.w700)),
					),
				],
			),
		);
		if (result != null && result.isNotEmpty) {
			setState(() => _poleHeight = result);
		}
	}
}

// ── Header ────────────────────────────────────────────────────────────────────

class _RoverHeader extends StatelessWidget {
	const _RoverHeader({required this.onBack, required this.rfMode});
	final VoidCallback onBack;
	final String rfMode;

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
						behavior: HitTestBehavior.opaque,
						child: Padding(
							padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
							child: Text('←',
									style: GoogleFonts.montserrat(
											fontSize: 24, color: Colors.white, height: 1)),
						),
					),
					const SizedBox(width: 16),
					Expanded(
						child: Text('Rover Config',
								style: GoogleFonts.montserrat(
										fontSize: 20,
										fontWeight: FontWeight.w700,
										color: Colors.white)),
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
							'RF: $rfMode',
							style: GoogleFonts.montserrat(
								fontSize: 11,
								fontWeight: FontWeight.w700,
								color: const Color(0xFF64B5F6),
							),
						),
					),
				],
			),
		);
	}
}

// ── Left panel ────────────────────────────────────────────────────────────────

class _LeftPanel extends StatelessWidget {
	const _LeftPanel({required this.receiving});
	final bool receiving;

	@override
	Widget build(BuildContext context) {
		return Container(
			width: 420,
			decoration: const BoxDecoration(
				color: Color(0xFFF8F9FA),
				border: Border(right: BorderSide(color: Color(0xFFE0E0E0))),
			),
			padding: const EdgeInsets.all(20),
			child: Column(
				crossAxisAlignment: CrossAxisAlignment.start,
				children: [
					_SectionLabel('RF Link'),
					const SizedBox(height: 10),
					Container(
						decoration: BoxDecoration(
							color: Colors.white,
							border: Border.all(color: const Color(0xFFE0E0E0)),
							borderRadius: BorderRadius.circular(8),
						),
						padding: const EdgeInsets.all(14),
						child: Column(
							children: [
								Row(
									children: [
										Container(
											width: 14,
											height: 14,
											decoration: BoxDecoration(
												color: receiving
														? const Color(0xFF43A047)
														: const Color(0xFFE0E0E0),
												shape: BoxShape.circle,
											),
										),
										const SizedBox(width: 12),
										Column(
											crossAxisAlignment:
													CrossAxisAlignment.start,
											children: [
												Text(
													receiving
															? 'Receiving RTCM'
															: 'Waiting for Base…',
													style: GoogleFonts.montserrat(
														fontSize: 14,
														fontWeight: FontWeight.w700,
														color: receiving
																? const Color(0xFF2E7D32)
																: const Color(0xFFBBBBBB),
													),
												),
												Text(
													receiving
															? '1,247 frames received'
															: '0 frames received',
													style: GoogleFonts.montserrat(
														fontSize: 11,
														color: receiving
																? const Color(0xFF888888)
																: const Color(0xFFCCCCCC),
													),
												),
											],
										),
									],
								),
								if (receiving) ...[
									const SizedBox(height: 10),
									_StatRow('RSSI', '−82 dBm'),
									_StatRow('SNR', '+8.5 dB'),
								],
							],
						),
					),
					const SizedBox(height: 18),
					_SectionLabel('F9P Status'),
					const SizedBox(height: 10),
					Container(
						decoration: BoxDecoration(
							color: Colors.white,
							border: Border.all(color: const Color(0xFFE0E0E0)),
							borderRadius: BorderRadius.circular(8),
						),
						padding: const EdgeInsets.all(14),
						child: Column(
							children: [
								Row(
									mainAxisAlignment:
											MainAxisAlignment.spaceBetween,
									children: [
										Text('Fix type',
												style: GoogleFonts.montserrat(
														fontSize: 13,
														color: const Color(0xFF888888))),
										_FixBadge(
											receiving: receiving,
										),
									],
								),
								const SizedBox(height: 8),
								_StatRow('Horiz. accuracy',
										receiving ? '± 0.9 cm' : '—',
										valueDim: !receiving),
								_StatRow('RTCM applied',
										receiving ? '1,247 messages' : '—',
										valueDim: !receiving),
							],
						),
					),
				],
			),
		);
	}
}

class _SectionLabel extends StatelessWidget {
	const _SectionLabel(this.text);
	final String text;

	@override
	Widget build(BuildContext context) {
		return Text(
			text.toUpperCase(),
			style: GoogleFonts.montserrat(
				fontSize: 11,
				fontWeight: FontWeight.w700,
				letterSpacing: 1.0,
				color: const Color(0xFF999999),
			),
		);
	}
}

class _StatRow extends StatelessWidget {
	const _StatRow(this.label, this.value, {this.valueDim = false});
	final String label;
	final String value;
	final bool valueDim;

	@override
	Widget build(BuildContext context) {
		return Padding(
			padding: const EdgeInsets.symmetric(vertical: 5),
			child: Row(
				mainAxisAlignment: MainAxisAlignment.spaceBetween,
				children: [
					Text(label,
							style: GoogleFonts.montserrat(
									fontSize: 13,
									color: const Color(0xFF888888))),
					Text(value,
							style: GoogleFonts.montserrat(
								fontSize: 14,
								fontWeight: FontWeight.w700,
								color: valueDim
										? const Color(0xFFCCCCCC)
										: kLightText,
							)),
				],
			),
		);
	}
}

class _FixBadge extends StatelessWidget {
	const _FixBadge({required this.receiving});
	final bool receiving;

	@override
	Widget build(BuildContext context) {
		return Container(
			padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 4),
			decoration: BoxDecoration(
				color: receiving
						? const Color(0xFFE8F5E9)
						: const Color(0xFFECEFF1),
				borderRadius: BorderRadius.circular(12),
			),
			child: Text(
				receiving ? 'RTK Fix' : 'No RTK',
				style: GoogleFonts.montserrat(
					fontSize: 12,
					fontWeight: FontWeight.w700,
					color: receiving
							? const Color(0xFF1B5E20)
							: const Color(0xFF607D8B),
				),
			),
		);
	}
}

// ── Right panel ───────────────────────────────────────────────────────────────

class _RightPanel extends StatelessWidget {
	const _RightPanel({
		required this.poleHeight,
		required this.receiving,
		required this.onToggle,
		required this.onPoleHeightEdit,
		required this.onSave,
	});

	final String poleHeight;
	final bool receiving;
	final VoidCallback onToggle;
	final VoidCallback onPoleHeightEdit;
	final VoidCallback onSave;

	@override
	Widget build(BuildContext context) {
		return Padding(
			padding: const EdgeInsets.fromLTRB(24, 20, 24, 20),
			child: Column(
				crossAxisAlignment: CrossAxisAlignment.start,
				children: [
					Expanded(
						child: SingleChildScrollView(
							child: Column(
								crossAxisAlignment: CrossAxisAlignment.start,
								children: [
					_SectionLabel('Pole Height'),
					const SizedBox(height: 10),
					GestureDetector(
						onTap: onPoleHeightEdit,
						child: Container(
							decoration: BoxDecoration(
								color: Colors.white,
								border: Border.all(
										color: const Color(0xFFE0E0E0), width: 2),
								borderRadius: BorderRadius.circular(6),
							),
							padding: const EdgeInsets.symmetric(
									horizontal: 14, vertical: 10),
							child: Row(
								mainAxisAlignment:
										MainAxisAlignment.spaceBetween,
								children: [
									Column(
										crossAxisAlignment:
												CrossAxisAlignment.start,
										children: [
											Text(
												'Ground mark → antenna phase centre',
												style: GoogleFonts.montserrat(
														fontSize: 12,
														color: const Color(0xFFAAAAAA)),
											),
											const SizedBox(height: 4),
											Text(
												'$poleHeight m',
												style: GoogleFonts.montserrat(
													fontSize: 22,
													fontWeight: FontWeight.w700,
													color: kLightText,
												),
											),
										],
									),
									Text('✎',
											style: TextStyle(
													fontSize: 16,
													color: const Color(0xFFBBBBBB))),
								],
							),
						),
					),
					const SizedBox(height: 18),

					_SectionLabel('Constellation'),
					const SizedBox(height: 10),
					Container(
						decoration: BoxDecoration(
							color: const Color(0xFFF5F5F5),
							border: Border.all(color: const Color(0xFFE0E0E0)),
							borderRadius: BorderRadius.circular(6),
						),
						padding: const EdgeInsets.symmetric(
								horizontal: 14, vertical: 10),
						child: Row(
							mainAxisAlignment:
									MainAxisAlignment.spaceBetween,
							children: [
								Text('RTCM constellation',
										style: GoogleFonts.montserrat(
												fontSize: 13,
												color: const Color(0xFF888888))),
								Text('GPS + GLONASS',
										style: GoogleFonts.montserrat(
											fontSize: 14,
											fontWeight: FontWeight.w700,
											color: const Color(0xFF555555),
										)),
							],
						),
					),
					const SizedBox(height: 4),
					Text(
						'Follows Base config — Rover F9P accepts all received RTCM types',
						style: GoogleFonts.montserrat(
								fontSize: 11,
								color: const Color(0xFFBBBBBB)),
					),
					const SizedBox(height: 18),

					_SectionLabel('System Mode'),
					const SizedBox(height: 10),
					Container(
						decoration: BoxDecoration(
							color: const Color(0xFFF5F5F5),
							border: Border.all(color: const Color(0xFFE0E0E0)),
							borderRadius: BorderRadius.circular(6),
						),
						padding: const EdgeInsets.symmetric(
								horizontal: 14, vertical: 10),
						child: Row(
							mainAxisAlignment:
									MainAxisAlignment.spaceBetween,
							children: [
								Text('Mode 1 — Local RF Link',
										style: GoogleFonts.montserrat(
												fontSize: 13,
												color: const Color(0xFF888888))),
								Text('Change ›',
										style: GoogleFonts.montserrat(
												fontSize: 11,
												color: const Color(0xFFBBBBBB))),
							],
						),
					),

								],
							),
						),
					),

					// Dev toggle
					Row(
						children: [
							Text('State:',
									style: GoogleFonts.montserrat(
											fontSize: 11, color: kLightTextMuted)),
							const SizedBox(width: 8),
							_ToggleBtn(
								label: '⏳ Waiting',
								active: !receiving,
								onTap: onToggle,
							),
							const SizedBox(width: 4),
							_ToggleBtn(
								label: '📡 Receiving',
								active: receiving,
								onTap: onToggle,
							),
						],
					),
					const SizedBox(height: 12),

					SizedBox(
						width: double.infinity,
						height: 48,
						child: ElevatedButton(
							onPressed: onSave,
							style: ElevatedButton.styleFrom(
								backgroundColor: kBlueButton,
								foregroundColor: Colors.white,
								elevation: 0,
								shape: RoundedRectangleBorder(
										borderRadius: BorderRadius.circular(8)),
							),
							child: Text('Save Settings',
									style: GoogleFonts.montserrat(
											fontSize: 15,
											fontWeight: FontWeight.w700)),
						),
					),
				],
			),
		);
	}
}

class _ToggleBtn extends StatelessWidget {
	const _ToggleBtn(
			{required this.label,
			required this.active,
			required this.onTap});
	final String label;
	final bool active;
	final VoidCallback onTap;

	@override
	Widget build(BuildContext context) {
		return GestureDetector(
			onTap: onTap,
			child: Container(
				padding: const EdgeInsets.symmetric(
						horizontal: 10, vertical: 4),
				decoration: BoxDecoration(
					color: active ? kBlueButton : const Color(0xFFF5F5F5),
					borderRadius: BorderRadius.circular(4),
					border: Border.all(
							color: active
									? kBlueButton
									: const Color(0xFFE0E0E0)),
				),
				child: Text(label,
						style: GoogleFonts.montserrat(
							fontSize: 11,
							fontWeight: FontWeight.w700,
							color:
									active ? Colors.white : kLightTextMuted,
						)),
			),
		);
	}
}
