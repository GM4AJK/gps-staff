import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';

class Mode5RemoteListenerScreen extends StatefulWidget {
	const Mode5RemoteListenerScreen({super.key});

	@override
	State<Mode5RemoteListenerScreen> createState() =>
			_Mode5RemoteListenerScreenState();
}

class _Mode5RemoteListenerScreenState
		extends State<Mode5RemoteListenerScreen> {
	bool _connected = false;
	String _listenPort = '2101';

	@override
	Widget build(BuildContext context) {
		return Scaffold(
			backgroundColor: kLightBg,
      resizeToAvoidBottomInset: false,
			body: Column(
				crossAxisAlignment: CrossAxisAlignment.stretch,
				children: [
					const StatusBar(),
					_Header(onBack: () => Navigator.pop(context)),
					Expanded(
						child: Row(
							crossAxisAlignment: CrossAxisAlignment.stretch,
							children: [
								_LeftPanel(connected: _connected),
								Expanded(
									child: _RightPanel(
										listenPort: _listenPort,
										connected: _connected,
										onToggle: () =>
												setState(() => _connected = !_connected),
										onPortEdit: () => _editPort(context),
										onSave: () {},
									),
								),
							],
						),
					),
				],
			),
		);
	}

	Future<void> _editPort(BuildContext context) async {
		final controller = TextEditingController(text: _listenPort);
		final result = await showDialog<String>(
			context: context,
			builder: (ctx) => AlertDialog(
				backgroundColor: kLightBg,
				title: Text('Listen Port',
						style: GoogleFonts.montserrat(
								fontWeight: FontWeight.w700, color: kLightText)),
				content: TextField(
					controller: controller,
					autofocus: true,
					keyboardType: TextInputType.number,
					style: GoogleFonts.montserrat(
							fontSize: 20,
							color: kLightText,
							fontFeatures: const [FontFeature.tabularFigures()]),
				),
				actions: [
					TextButton(
						onPressed: () => Navigator.pop(ctx),
						child: Text('Cancel',
								style: GoogleFonts.montserrat(color: kLightTextMuted)),
					),
					ElevatedButton(
						onPressed: () => Navigator.pop(ctx, controller.text),
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
			setState(() => _listenPort = result);
		}
	}
}

// ── Header ────────────────────────────────────────────────────────────────────

class _Header extends StatelessWidget {
	const _Header({required this.onBack});
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
						padding:
								const EdgeInsets.symmetric(horizontal: 10, vertical: 3),
						decoration: BoxDecoration(
							color: const Color(0xFF1A0A2E),
							border: Border.all(color: const Color(0xFF4A148C)),
							borderRadius: BorderRadius.circular(4),
						),
						child: Text(
							'Mode 5 · Remote',
							style: GoogleFonts.montserrat(
								fontSize: 11,
								fontWeight: FontWeight.w700,
								color: const Color(0xFFCE93D8),
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
	const _LeftPanel({required this.connected});
	final bool connected;

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
					_SectionLabel('TCP Connection'),
					const SizedBox(height: 10),
					Container(
						decoration: BoxDecoration(
							color: Colors.white,
							border: Border.all(color: const Color(0xFFE0E0E0)),
							borderRadius: BorderRadius.circular(8),
						),
						padding: const EdgeInsets.all(16),
						child: Column(
							crossAxisAlignment: CrossAxisAlignment.start,
							children: [
								Row(
									children: [
										Container(
											width: 14,
											height: 14,
											decoration: BoxDecoration(
												color: connected
														? const Color(0xFFE8F5E9)
														: const Color(0xFFFFF8E1),
												shape: BoxShape.circle,
												border: Border.all(
													color: connected
															? const Color(0xFF43A047)
															: const Color(0xFFFFB300),
													width: 2,
												),
											),
										),
										const SizedBox(width: 12),
										Column(
											crossAxisAlignment:
													CrossAxisAlignment.start,
											children: [
												Text(
													connected
															? 'Client connected'
															: 'Listening on port 2101',
													style: GoogleFonts.montserrat(
														fontSize: 14,
														fontWeight: FontWeight.w700,
														color: connected
																? const Color(0xFF2E7D32)
																: const Color(0xFFFF8F00),
													),
												),
												Text(
													connected
															? 'Streaming RTCM corrections'
															: 'Waiting for mobile device to connect…',
													style: GoogleFonts.montserrat(
														fontSize: 11,
														color: const Color(0xFFBBBBBB),
													),
												),
											],
										),
									],
								),
								if (connected) ...[
									const SizedBox(height: 12),
									_StatRow('Client address', '203.0.113.42'),
									_StatRow('RTCM sent', '2,847,392 bytes'),
									_StatRow('Connected for', '00:08:24'),
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
						padding: const EdgeInsets.fromLTRB(14, 14, 14, 14),
						child: Column(
							children: [
								_StatRow(
									'Fix type',
									connected ? 'RTK Fix' : 'No RTK',
									valueColor: connected
											? const Color(0xFF2E7D32)
											: const Color(0xFF607D8B),
								),
								_StatRow(
									'Horiz. accuracy',
									connected ? '± 0.8 cm' : '—',
									valueDim: !connected,
								),
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
	const _StatRow(this.label, this.value,
			{this.valueDim = false, this.valueColor});
	final String label;
	final String value;
	final bool valueDim;
	final Color? valueColor;

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
					Text(
						value,
						style: GoogleFonts.montserrat(
							fontSize: 14,
							fontWeight: FontWeight.w700,
							color: valueColor ??
									(valueDim
											? const Color(0xFFCCCCCC)
											: kLightText),
						),
					),
				],
			),
		);
	}
}

// ── Right panel ───────────────────────────────────────────────────────────────

class _RightPanel extends StatelessWidget {
	const _RightPanel({
		required this.listenPort,
		required this.connected,
		required this.onToggle,
		required this.onPortEdit,
		required this.onSave,
	});

	final String listenPort;
	final bool connected;
	final VoidCallback onToggle;
	final VoidCallback onPortEdit;
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
									_SectionLabel('Listen Port'),
									const SizedBox(height: 10),
									GestureDetector(
										onTap: onPortEdit,
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
																'TCP port this device listens on',
																style: GoogleFonts.montserrat(
																		fontSize: 12,
																		color: const Color(0xFFAAAAAA)),
															),
															const SizedBox(height: 4),
															Text(
																listenPort,
																style: GoogleFonts.montserrat(
																	fontSize: 22,
																	fontWeight: FontWeight.w700,
																	color: kLightText,
																	fontFeatures: const [
																		FontFeature.tabularFigures()
																	],
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
									const SizedBox(height: 8),
									Container(
										decoration: BoxDecoration(
											color: const Color(0xFFFAFAFA),
											border: Border.all(color: const Color(0xFFEFEFEF)),
											borderRadius: BorderRadius.circular(6),
										),
										padding: const EdgeInsets.all(10),
										child: RichText(
											text: TextSpan(
												style: GoogleFonts.montserrat(
														fontSize: 11,
														color: const Color(0xFFAAAAAA),
														height: 1.6),
												children: [
													TextSpan(
														text: 'Router setup: ',
														style: GoogleFonts.montserrat(
															fontSize: 11,
															fontWeight: FontWeight.w700,
															color: const Color(0xFF888888),
														),
													),
													const TextSpan(
														text:
																'Forward this port on your home router to this device\'s local IP address. '
																'The mobile device connects using your router\'s external hostname and this port number.',
													),
												],
											),
										),
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
												Column(
													crossAxisAlignment:
															CrossAxisAlignment.start,
													children: [
														Text('Mode 5 — Direct TCP',
																style: GoogleFonts.montserrat(
																		fontSize: 13,
																		color: const Color(0xFF888888))),
														Text('This device: remote listener',
																style: GoogleFonts.montserrat(
																		fontSize: 11,
																		color: const Color(0xFFBBBBBB))),
													],
												),
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

					const SizedBox(height: 12),
					Row(
						children: [
							Text('State:',
									style: GoogleFonts.montserrat(
											fontSize: 11, color: kLightTextMuted)),
							const SizedBox(width: 8),
							_ToggleBtn(
								label: '⏳ Idle',
								active: !connected,
								onTap: onToggle,
							),
							const SizedBox(width: 4),
							_ToggleBtn(
								label: '🔗 Connected',
								active: connected,
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
							child: Text('Save & Start Listening',
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
