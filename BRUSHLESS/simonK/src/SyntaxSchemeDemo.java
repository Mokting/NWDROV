import java.awt.*;
import java.awt.event.*;
import java.io.IOException;
import javax.swing.*;

import org.fife.ui.rtextarea.*;
import org.fife.ui.rsyntaxtextarea.*;

/**
 * A simple example showing how to modify the fonts and colors used in an
 * RSyntaxTextArea. There are two methods to do this - via the Java API, and via
 * an XML file. The latter method is preferred since it's more modular, and
 * provides a way for your users to customize RSTA in your application.<p>
 * 
 * This example uses RSyntaxTextArea 2.0.1.<p>
 * 
 * Project Home: http://fifesoft.com/rsyntaxtextarea<br>
 * Downloads: https://sourceforge.net/projects/rsyntaxtextarea
 */
public class SyntaxSchemeDemo extends JFrame implements ActionListener {

   private static final long serialVersionUID = 1L;

   private RSyntaxTextArea textArea;
   private Theme theme;

   private static final String text = ".equ VARIABLE = 1000 ;comment\n" +
   		".include \"m8def.inc\"\n" +
   		"public class ExampleSource {\n\n"
         + "   // Check out the crazy modified styles!\n"
         + "   public static void main(String[] args) {\n"
         + "      System.out.println(\"Hello, world!\");\n" + "   }\n\n"
         + "}\n";

   public SyntaxSchemeDemo() {

      JPanel cp = new JPanel(new BorderLayout());

      textArea = new RSyntaxTextArea(20, 60);
      textArea.setSyntaxEditingStyle(SyntaxConstants.SYNTAX_STYLE_ASSEMBLER_X86);
      textArea.setCodeFoldingEnabled(true);
      textArea.setAntiAliasingEnabled(true);
     
      try {
    	  theme = Theme.load(getClass().getResourceAsStream("/de/lazyzero/kkMulticopterFlashTool/gui/resources/dark.xml"));
    	  theme.apply(textArea);
      } catch (IOException e) {
    	  // TODO Auto-generated catch block
    	  e.printStackTrace();
      }
      RTextScrollPane sp = new RTextScrollPane(textArea);
      cp.add(sp);

      textArea.setText(text);

      JMenuBar mb = new JMenuBar();
      JMenu menu = new JMenu("File");
      mb.add(menu);
      JMenuItem changeStyleProgrammaticallyItem = new JMenuItem(
            "Change Style Programmatically");
      changeStyleProgrammaticallyItem
            .setActionCommand("ChangeProgrammatically");
      changeStyleProgrammaticallyItem.addActionListener(this);
      menu.add(changeStyleProgrammaticallyItem);
      JMenuItem changeStyleViaThemesItem = new JMenuItem(
            "Change Style via Theme XML");
      changeStyleViaThemesItem.setActionCommand("ChangeViaThemes");
      changeStyleViaThemesItem.addActionListener(this);
      menu.add(changeStyleViaThemesItem);
      
      mb.add(new JMenuItem(RTextArea.getAction(RTextArea.UNDO_ACTION)));
      mb.add(new JMenuItem(RTextArea.getAction(RTextArea.REDO_ACTION)));
      
      setJMenuBar(mb);

      setContentPane(cp);
      setTitle("Syntax Scheme Demo");
      setDefaultCloseOperation(EXIT_ON_CLOSE);
      pack();
      setLocationRelativeTo(null);

   }

   /**
    * Listens for the selection of a menu item and performs an action
    * accordingly.
    */
   public void actionPerformed(ActionEvent e) {
      String command = e.getActionCommand();
      if ("ChangeProgrammatically".equals(command)) {
         changeStyleProgrammatically();
      } else if ("ChangeViaThemes".equals(command)) {
         changeStyleViaThemeXml();
      } 
   }

   /**
    * Changes the styles used in the editor programmatically.
    */
   private void changeStyleProgrammatically() {

      // Set the font for all token types.
      setFont(textArea, new Font("Comic Sans MS", Font.PLAIN, 16));

      // Change a few things here and there.
      SyntaxScheme scheme = textArea.getSyntaxScheme();
      scheme.getStyle(Token.RESERVED_WORD).background = Color.pink;
      scheme.getStyle(Token.DATA_TYPE).foreground = Color.blue;
      scheme.getStyle(Token.LITERAL_STRING_DOUBLE_QUOTE).underline = true;
      scheme.getStyle(Token.COMMENT_EOL).font = new Font("Georgia",
            Font.ITALIC, 12);

      textArea.revalidate();

   }

   /**
    * Changes the styles used by the editor via an XML file specification. This
    * method is preferred because of its ease and modularity.
    */
   private void changeStyleViaThemeXml() {
      try {
         Theme theme = Theme.load(getClass().getResourceAsStream(
               "/dark.xml"));
         theme.apply(textArea);
      } catch (IOException ioe) { // Never happens
         ioe.printStackTrace();
      }
   }

   /**
    * Set the font for all token types.
    * 
    * @param textArea The text area to modify.
    * @param font The font to use.
    */
   public static void setFont(RSyntaxTextArea textArea, Font font) {
      if (font != null) {
         SyntaxScheme ss = textArea.getSyntaxScheme();
         ss = (SyntaxScheme) ss.clone();
         for (int i = 0; i < ss.getStyleCount(); i++) {
            if (ss.getStyle(i) != null) {
               ss.getStyle(i).font = font;
            }
         }
         textArea.setSyntaxScheme(ss);
         textArea.setFont(font);
      }
   }

   public static void main(String[] args) {
      // Start all Swing applications on the EDT.
      SwingUtilities.invokeLater(new Runnable() {
         public void run() {
            new SyntaxSchemeDemo().setVisible(true);
         }
      });
   }

}