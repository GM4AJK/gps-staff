import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';
import 'rover_working_screen.dart';

class StartSurveySessionScreen extends StatefulWidget {
	const StartSurveySessionScreen({super.key});

	@override
	State<StartSurveySessionScreen> createState() =>
			_StartSurveySessionScreenState();
}

class _StartSurveySessionScreenState
		extends State<StartSurveySessionScreen> {
	bool _sdPresent = true;
	String _filename = 'survey_004';
	String _title = '';
	String _description = '';

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
								_LeftPanel(
									sdPresent: _sdPresent,
									onToggle: () =>
											setState(() => _sdPresent = !_sdPresent),
								),
								Expanded(
									child: _RightPanel(
										filename: _filename,
										title: _title,
										description: _description,
										sdPresent: _sdPresent,
										onFilenameEdit: () => _editField(
											context,
											label: 'Filename',
											hint: 'survey_004',
											initial: _filename,
											onDone: (v) =>
													setState(() => _filename = v),
										),
										onTitleEdit: () => _editField(
											context,
											label: 'Title',
											hint: 'e.g. Garden CP survey',
											initial: _title,
											onDone: (v) =>
													setState(() => _title = v),
										),
										onDescEdit: () => _editField(
											context,
											label: 'Description',
											hint:
													'Base setup, weather conditions, point identifiers…',
											initial: _description,
											multiLine: true,
											onDone: (v) =>
													setState(() => _description = v),
										),
										onStart: _sdPresent
												? () => Navigator.push(
														context,
														MaterialPageRoute(
															builder: (_) =>
																	RoverWorkingScreen(
																		sessionName: _filename,
																		sessionTitle: _title.isEmpty
																				? 'Untitled session'
																				: _title,
																	),
														),
													)
												: null,
									),
								),
							],
						),
					),
				],
			),
		);
	}

	Future<void> _editField(
		BuildContext context, {
		required String label,
		required String hint,
		required String initial,
		bool multiLine = false,
		required ValueChanged<String> onDone,
	}) async {
		final controller = TextEditingController(text: initial);
		final result = await showDialog<String>(
			context: context,
			builder: (ctx) => AlertDialog(
				backgroundColor: kLightBg,
				title: Text(label,
						style: GoogleFonts.montserrat(
								fontWeight: FontWeight.w700, color: kLightText)),
				content: TextField(
					controller: controller,
					autofocus: true,
					maxLines: multiLine ? 4 : 1,
					decoration: InputDecoration(
						hintText: hint,
						hintStyle: GoogleFonts.montserrat(color: kLightTextMuted),
					),
					style: GoogleFonts.montserrat(fontSize: 14, color: kLightText),
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
		if (result != null) onDone(result);
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
			padding: const EdgeInsets.symmetric(horizontal: 16),
			decoration: const BoxDecoration(
				color: kLightBg,
				border: Border(bottom: BorderSide(color: Color(0xFFE0E0E0))),
			),
			child: Row(
				children: [
					GestureDetector(
						onTap: onBack,
						behavior: HitTestBehavior.opaque,
						child: Padding(
							padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
							child: Text('←',
									style: GoogleFonts.montserrat(
											fontSize: 24,
											color: const Color(0xFFAAAAAA),
											height: 1)),
						),
					),
					const SizedBox(width: 14),
					Text('New Survey Session',
							style: GoogleFonts.montserrat(
								fontSize: 20,
								fontWeight: FontWeight.w700,
								color: const Color(0xFF222222),
							)),
				],
			),
		);
	}
}

// ── Left panel ────────────────────────────────────────────────────────────────

class _LeftPanel extends StatelessWidget {
	const _LeftPanel({required this.sdPresent, required this.onToggle});
	final bool sdPresent;
	final VoidCallback onToggle;

	@override
	Widget build(BuildContext context) {
		return Container(
			width: 300,
			decoration: const BoxDecoration(
				color: Color(0xFFF8F9FA),
				border: Border(right: BorderSide(color: Color(0xFFE0E0E0))),
			),
			padding: const EdgeInsets.all(20),
			child: Column(
				crossAxisAlignment: CrossAxisAlignment.start,
				children: [
					Expanded(
						child: SingleChildScrollView(
							child: Column(
								crossAxisAlignment: CrossAxisAlignment.start,
								children: [
					_SectionLabel('SD Card'),
					const SizedBox(height: 10),
					Container(
						decoration: BoxDecoration(
							color: Colors.white,
							border: Border.all(color: const Color(0xFFE0E0E0)),
							borderRadius: BorderRadius.circular(8),
						),
						padding: const EdgeInsets.all(14),
						child: Column(
							crossAxisAlignment: CrossAxisAlignment.start,
							children: [
								Row(
									children: [
										Container(
											width: 12,
											height: 12,
											decoration: BoxDecoration(
												color: sdPresent
														? const Color(0xFF43A047)
														: const Color(0xFFE53935),
												shape: BoxShape.circle,
											),
										),
										const SizedBox(width: 10),
										Text(
											sdPresent
													? 'Card detected'
													: 'No card detected',
											style: GoogleFonts.montserrat(
												fontSize: 14,
												fontWeight: FontWeight.w700,
												color: sdPresent
														? const Color(0xFF2E7D32)
														: const Color(0xFFC62828),
											),
										),
									],
								),
								if (sdPresent) ...[
									const SizedBox(height: 10),
									Text(
										'Available: 14.2 GB free\nSessions on card: 3',
										style: GoogleFonts.montserrat(
											fontSize: 12,
											color: const Color(0xFF888888),
											height: 1.7,
										),
									),
								],
							],
						),
					),
					const SizedBox(height: 16),
					_SectionLabel('This Session'),
					const SizedBox(height: 10),
					RichText(
						text: TextSpan(
							style: GoogleFonts.montserrat(
									fontSize: 13,
									color: const Color(0xFF888888),
									height: 1.8),
							children: [
								TextSpan(
									text: 'Session #004\n',
									style: GoogleFonts.montserrat(
										fontSize: 13,
										fontWeight: FontWeight.w700,
										color: kLightText,
									),
								),
								const TextSpan(text: 'File: '),
								TextSpan(
									text: 'survey_004.csv\n',
									style: GoogleFonts.montserrat(
										fontSize: 13,
										color: const Color(0xFF555555),
									),
								),
								const TextSpan(text: 'Pole height: '),
								TextSpan(
									text: '1.80 m\n',
									style: GoogleFonts.montserrat(
										fontSize: 13,
										fontWeight: FontWeight.w700,
										color: kLightText,
									),
								),
								const TextSpan(text: 'Mode: 1 — Local RF'),
							],
						),
					),
								],
							),
						),
					),

					if (!sdPresent)
						Container(
							padding: const EdgeInsets.all(10),
							decoration: BoxDecoration(
								color: const Color(0xFFFFF3E0),
								border: Border.all(color: const Color(0xFFFFB300)),
								borderRadius: BorderRadius.circular(6),
							),
							child: Text(
								'Insert SD card to save session data. Without a card, position data will not be logged.',
								style: GoogleFonts.montserrat(
									fontSize: 12,
									color: const Color(0xFFE65100),
									height: 1.6,
								),
							),
						),

					const SizedBox(height: 12),
					// Dev toggle
					Row(
						children: [
							Text('SD:',
									style: GoogleFonts.montserrat(
											fontSize: 11, color: kLightTextMuted)),
							const SizedBox(width: 6),
							_ToggleBtn(
								label: '💾 Present',
								active: sdPresent,
								onTap: () {
									if (!sdPresent) onToggle();
								},
							),
							const SizedBox(width: 4),
							_ToggleBtn(
								label: '✗ Absent',
								active: !sdPresent,
								onTap: () {
									if (sdPresent) onToggle();
								},
							),
						],
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

// ── Right panel ───────────────────────────────────────────────────────────────

class _RightPanel extends StatelessWidget {
	const _RightPanel({
		required this.filename,
		required this.title,
		required this.description,
		required this.sdPresent,
		required this.onFilenameEdit,
		required this.onTitleEdit,
		required this.onDescEdit,
		required this.onStart,
	});

	final String filename;
	final String title;
	final String description;
	final bool sdPresent;
	final VoidCallback onFilenameEdit;
	final VoidCallback onTitleEdit;
	final VoidCallback onDescEdit;
	final VoidCallback? onStart;

	@override
	Widget build(BuildContext context) {
		return Padding(
			padding: const EdgeInsets.fromLTRB(28, 20, 28, 20),
			child: Column(
				crossAxisAlignment: CrossAxisAlignment.start,
				children: [
					Expanded(
						child: SingleChildScrollView(
							child: Column(
								crossAxisAlignment: CrossAxisAlignment.start,
								children: [
					_FieldWrap(
						label: 'Filename',
						sublabel: '— .csv extension added automatically',
						child: GestureDetector(
							onTap: onFilenameEdit,
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
										Row(
											children: [
												Text(filename,
														style: GoogleFonts.montserrat(
															fontSize: 14,
															color: kLightText,
														)),
												Container(
													margin: const EdgeInsets.only(left: 8),
													padding: const EdgeInsets.symmetric(
															horizontal: 8, vertical: 2),
													decoration: BoxDecoration(
														color:
																const Color(0xFFF5F5F5),
														border: Border.all(
																color: const Color(
																		0xFFE0E0E0)),
														borderRadius:
																BorderRadius.circular(4),
													),
													child: Text('.csv',
															style: GoogleFonts.montserrat(
																fontSize: 12,
																fontWeight: FontWeight.w700,
																color: const Color(0xFFAAAAAA),
															)),
												),
											],
										),
										Text('✎',
												style: TextStyle(
														fontSize: 15,
														color: const Color(0xFFBBBBBB))),
									],
								),
							),
						),
					),
					Text(
						'FAT-safe characters only: a–z, 0–9, underscore, hyphen. Max 20 chars.',
						style: GoogleFonts.montserrat(
								fontSize: 11, color: const Color(0xFFBBBBBB)),
					),
					const SizedBox(height: 14),

					_FieldWrap(
						label: 'Title',
						sublabel: '— required',
						child: GestureDetector(
							onTap: onTitleEdit,
							child: _FieldBox(
								value: title.isEmpty ? null : title,
								placeholder: 'e.g. Garden CP survey',
							),
						),
					),
					const SizedBox(height: 14),

					_FieldWrap(
						label: 'Description',
						sublabel: '— optional',
						child: GestureDetector(
							onTap: onDescEdit,
							child: _FieldBox(
								value: description.isEmpty
										? null
										: description,
								placeholder:
										'Base setup, weather conditions, point identifiers…',
								multiLine: true,
							),
						),
					),
								],
							),
						),
					),

					Row(
						children: [
							Expanded(
								child: ElevatedButton(
									onPressed: onStart,
									style: ElevatedButton.styleFrom(
										backgroundColor: onStart != null
												? const Color(0xFF2E7D32)
												: const Color(0xFFBDBDBD),
										foregroundColor: Colors.white,
										elevation: 0,
										minimumSize: const Size(double.infinity, 52),
										tapTargetSize: MaterialTapTargetSize.shrinkWrap,
										shape: RoundedRectangleBorder(
												borderRadius: BorderRadius.circular(8)),
									),
									child: Text(
										onStart != null
												? 'Start Session →'
												: 'SD Card Required',
										style: GoogleFonts.montserrat(
												fontSize: 16,
												fontWeight: FontWeight.w700),
									),
								),
							),
						],
					),
				],
			),
		);
	}
}

class _FieldWrap extends StatelessWidget {
	const _FieldWrap({
		required this.label,
		required this.sublabel,
		required this.child,
	}) : expand = false;

	final String label;
	final String sublabel;
	final Widget child;
	final bool expand;

	@override
	Widget build(BuildContext context) {
		final content = Column(
			crossAxisAlignment: CrossAxisAlignment.start,
			children: [
				RichText(
					text: TextSpan(
						children: [
							TextSpan(
								text: label.toUpperCase(),
								style: GoogleFonts.montserrat(
									fontSize: 11,
									fontWeight: FontWeight.w700,
									letterSpacing: 0.8,
									color: const Color(0xFF999999),
								),
							),
							TextSpan(
								text: sublabel,
								style: GoogleFonts.montserrat(
									fontSize: 11,
									color: const Color(0xFFCCCCCC),
								),
							),
						],
					),
				),
				const SizedBox(height: 4),
				if (expand) Expanded(child: child) else child,
			],
		);

		return expand
				? Expanded(child: content)
				: content;
	}
}

class _FieldBox extends StatelessWidget {
	const _FieldBox({this.value, required this.placeholder, this.multiLine = false});
	final String? value;
	final String placeholder;
	final bool multiLine;

	@override
	Widget build(BuildContext context) {
		return Container(
			width: double.infinity,
			constraints: multiLine
					? const BoxConstraints(minHeight: 72)
					: const BoxConstraints(minHeight: 44),
			decoration: BoxDecoration(
				color: Colors.white,
				border: Border.all(color: const Color(0xFFE0E0E0), width: 2),
				borderRadius: BorderRadius.circular(6),
			),
			padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
			child: Row(
				crossAxisAlignment: multiLine
						? CrossAxisAlignment.start
						: CrossAxisAlignment.center,
				children: [
					Expanded(
						child: Text(
							value ?? placeholder,
							style: GoogleFonts.montserrat(
								fontSize: multiLine ? 13 : 14,
								color: value != null
										? kLightText
										: const Color(0xFFCCCCCC),
								fontStyle: value != null
										? FontStyle.normal
										: FontStyle.italic,
								height: multiLine ? 1.6 : 1,
							),
						),
					),
					Text('✎',
							style: TextStyle(
									fontSize: 15,
									color: const Color(0xFFBBBBBB))),
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
